package com.mtk.shell;

import android.app.Notification;
import android.content.ComponentName;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.service.notification.NotificationListenerService;
import android.service.notification.StatusBarNotification;
import android.util.AtomicFile;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.RandomAccessFile;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Date;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;

/** Records every notification post/update as one physical UTF-8 line. */
public class NotificationLogService extends NotificationListenerService {
    private static final String LOG_FILE_NAME = "notifications.txt";
    private static final int MAX_FIELD_LENGTH = 16 * 1024;
    // TextAdapter also limits a file to 100,000 lines. At 2 MiB even the
    // shortest possible record stays comfortably below that count.
    private static final long MAX_ACTIVE_LOG_BYTES = 2L * 1024L * 1024L;
    private static final int MAX_ARCHIVE_FILES = 16;
    private static final Object FILE_LOCK = new Object();
    private static long logGeneration;

    private final ExecutorService writer = Executors.newSingleThreadExecutor(runnable -> {
        Thread thread = new Thread(runnable, "ClearShell-notification-log");
        thread.setPriority(Thread.MIN_PRIORITY);
        return thread;
    });

    public static File getLogFile(Context context) {
        return new File(context.getNoBackupFilesDir(), LOG_FILE_NAME);
    }

    static Object fileLock() {
        return FILE_LOCK;
    }

    static boolean isLogFile(Context context, File file) {
        return context != null && file != null
                && getLogFile(context).getAbsolutePath().equals(file.getAbsolutePath());
    }

    /** Completes recovery left by an interrupted AtomicFile rewrite. */
    static void recoverLogFile(Context context) {
        File logFile = getLogFile(context);
        try (FileInputStream ignored = new AtomicFile(logFile).openRead()) {
            // Opening is enough: AtomicFile restores a valid .bak before reading.
        } catch (Throwable ignored) {
            // A new installation legitimately has no log file yet.
        }
    }

    static long logGeneration() {
        return logGeneration;
    }

    static void ensureConnected(Context context) {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
            try {
                requestRebind(new ComponentName(context, NotificationLogService.class));
            } catch (Throwable ignored) {
            }
        }
    }

    @Override
    public void onNotificationPosted(StatusBarNotification statusBarNotification) {
        if (statusBarNotification == null) {
            return;
        }
        final long postTime = statusBarNotification.getPostTime();
        final String packageName = safe(statusBarNotification.getPackageName());
        Notification notification = statusBarNotification.getNotification();
        Bundle extras = notification == null ? null : notification.extras;
        final String title = notificationTitle(extras);
        final String text = notificationText(extras);
        try {
            writer.execute(() -> appendNotification(postTime, packageName, title, text));
        } catch (RejectedExecutionException ignored) {
            // The service is already being disconnected.
        }
    }

    private void appendNotification(long postTime, String packageName,
                                    String title, String text) {
        String appName = packageName;
        try {
            PackageManager packageManager = getPackageManager();
            ApplicationInfo applicationInfo = packageManager.getApplicationInfo(packageName, 0);
            CharSequence label = packageManager.getApplicationLabel(applicationInfo);
            if (label != null && label.length() > 0) {
                appName = label.toString();
            }
        } catch (Throwable ignored) {
        }

        String timestamp = new SimpleDateFormat(
                "yyyy-MM-dd HH:mm:ss.SSS", Locale.US).format(new Date(postTime));
        String line = escape(timestamp) + '\t'
                + escape(appName) + " [" + escape(packageName) + "]" + '\t'
                + escape(title) + '\t' + escape(text) + '\n';
        byte[] bytes = line.getBytes(StandardCharsets.UTF_8);

        synchronized (FILE_LOCK) {
            File logFile = getLogFile(this);
            recoverLogFile(this);
            File parent = logFile.getParentFile();
            if (parent != null && !parent.exists() && !parent.mkdirs()) {
                return;
            }
            boolean separatorNeeded = needsLineSeparator(logFile);
            archiveFullLog(logFile, bytes.length + (separatorNeeded ? 1 : 0));
            separatorNeeded = needsLineSeparator(logFile);
            try (FileOutputStream output = new FileOutputStream(logFile, true)) {
                if (separatorNeeded) {
                    output.write('\n');
                }
                output.write(bytes);
                output.flush();
            } catch (Throwable ignored) {
                // A notification callback must never destabilize the listener service.
            }
        }
    }

    private static boolean needsLineSeparator(File logFile) {
        if (!logFile.isFile() || logFile.length() == 0) {
            return false;
        }
        try (RandomAccessFile input = new RandomAccessFile(logFile, "r")) {
            input.seek(input.length() - 1);
            return input.read() != '\n';
        } catch (Throwable ignored) {
            return true;
        }
    }

    private static void archiveFullLog(File logFile, int bytesToAppend) {
        if (!logFile.isFile()
                || logFile.length() + bytesToAppend <= MAX_ACTIVE_LOG_BYTES) {
            return;
        }
        File parent = logFile.getParentFile();
        if (parent == null) {
            return;
        }
        String timestamp = new SimpleDateFormat(
                "yyyyMMdd-HHmmss-SSS", Locale.US).format(new Date());
        File archive = new File(parent, "notifications-" + timestamp + ".txt");
        int duplicate = 1;
        while (archive.exists()) {
            archive = new File(parent,
                    "notifications-" + timestamp + '-' + duplicate + ".txt");
            duplicate++;
        }
        // Rotate before reaching either of TextAdapter's hard limits.
        if (logFile.renameTo(archive)) {
            logGeneration++;
            pruneArchives(parent);
            return;
        }

        // Same-directory rename should normally be atomic. Keep a copy/truncate
        // fallback for unusual OEM filesystems rather than letting the active
        // log grow beyond the viewer's hard limit.
        boolean copied = false;
        try (FileInputStream input = new FileInputStream(logFile);
             FileOutputStream output = new FileOutputStream(archive)) {
            byte[] buffer = new byte[64 * 1024];
            int count;
            while ((count = input.read(buffer)) != -1) {
                output.write(buffer, 0, count);
            }
            output.flush();
            output.getFD().sync();
            copied = true;
        } catch (Throwable ignored) {
        }
        if (!copied) {
            archive.delete();
            return;
        }
        boolean truncated = false;
        try (FileOutputStream ignored = new FileOutputStream(logFile, false)) {
            // Truncate only after the archive is fully persisted.
            truncated = true;
        } catch (Throwable ignored) {
            // Both copies remain intact; a later notification can retry rotation.
        }
        if (truncated) {
            logGeneration++;
            pruneArchives(parent);
        }
    }

    private static void pruneArchives(File parent) {
        File[] archives = parent.listFiles((directory, name) ->
                name.startsWith("notifications-") && name.endsWith(".txt"));
        if (archives == null || archives.length <= MAX_ARCHIVE_FILES) {
            return;
        }
        Arrays.sort(archives, new Comparator<File>() {
            @Override
            public int compare(File first, File second) {
                return first.getName().compareTo(second.getName());
            }
        });
        for (int i = 0; i < archives.length - MAX_ARCHIVE_FILES; i++) {
            archives[i].delete();
        }
    }

    private static String notificationTitle(Bundle extras) {
        if (extras == null) {
            return "";
        }
        CharSequence title = extras.getCharSequence(Notification.EXTRA_TITLE);
        if (title == null || title.length() == 0) {
            title = extras.getCharSequence(Notification.EXTRA_TITLE_BIG);
        }
        return safe(title);
    }

    private static String notificationText(Bundle extras) {
        if (extras == null) {
            return "";
        }
        CharSequence text = extras.getCharSequence(Notification.EXTRA_BIG_TEXT);
        if (text == null || text.length() == 0) {
            CharSequence[] lines = extras.getCharSequenceArray(Notification.EXTRA_TEXT_LINES);
            if (lines != null && lines.length > 0) {
                StringBuilder joined = new StringBuilder();
                for (CharSequence line : lines) {
                    if (line == null || line.length() == 0) {
                        continue;
                    }
                    if (joined.length() > 0) {
                        joined.append(" | ");
                    }
                    joined.append(line);
                }
                text = joined;
            }
        }
        if (text == null || text.length() == 0) {
            text = extras.getCharSequence(Notification.EXTRA_TEXT);
        }
        return safe(text);
    }

    private static String safe(Object value) {
        if (value == null) {
            return "";
        }
        String text = value.toString();
        if (text.length() > MAX_FIELD_LENGTH) {
            return text.substring(0, MAX_FIELD_LENGTH) + "…";
        }
        return text;
    }

    private static String escape(String value) {
        String text = safe(value);
        StringBuilder escaped = new StringBuilder(text.length() + 16);
        for (int i = 0; i < text.length(); i++) {
            char character = text.charAt(i);
            switch (character) {
                case '\\':
                    escaped.append("\\\\");
                    break;
                case '\n':
                    escaped.append("\\n");
                    break;
                case '\r':
                    escaped.append("\\r");
                    break;
                case '\t':
                    escaped.append("\\t");
                    break;
                default:
                    escaped.append(character < 0x20 ? ' ' : character);
                    break;
            }
        }
        return escaped.toString();
    }

    @Override
    public void onDestroy() {
        writer.shutdown();
        super.onDestroy();
    }
}
