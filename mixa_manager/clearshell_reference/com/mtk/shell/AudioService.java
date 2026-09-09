package com.mtk.shell;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.IBinder;

import androidx.core.app.NotificationCompat;
import androidx.core.content.ContextCompat;

import java.io.File;

public class AudioService extends Service {

    private static final String CHANNEL_ID = "clearshell_audio";
    private static final String ACTION_KEEP_ALIVE = "com.mtk.shell.action.KEEP_AUDIO_ALIVE";
    private static final int NOTIFICATION_ID = 1001;

    private AudioServiceBinder audioServiceBinder;
    private boolean foreground;

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        audioServiceBinder = new AudioServiceBinder(this, new AudioServiceBinder.PlaybackListener() {
            @Override
            public void onPlaybackStarted(File file) {
                showPlaybackNotification(file);
            }

            @Override
            public void onPlaybackStopped() {
                stopPlaybackForeground();
            }
        });
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return audioServiceBinder;
    }

    @Override
    public void onDestroy() {
        if (audioServiceBinder != null) {
            audioServiceBinder.release();
        }
        removeForegroundNotification();
        super.onDestroy();
    }

    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                    CHANNEL_ID,
                    getString(R.string.audio_notification_channel),
                    NotificationManager.IMPORTANCE_LOW);
            channel.setDescription(getString(R.string.audio_notification_playing));
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }
    }

    private Notification buildNotification(File file) {
        int pendingIntentFlags = PendingIntent.FLAG_UPDATE_CURRENT;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            pendingIntentFlags |= PendingIntent.FLAG_IMMUTABLE;
        }

        Intent openIntent = new Intent(this, ClearShell.class)
                .setAction(Intent.ACTION_MAIN)
                .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent openPendingIntent = PendingIntent.getActivity(
                this, 0, openIntent, pendingIntentFlags);

        String text = file == null
                ? getString(R.string.audio_notification_playing)
                : file.getName();
        return new NotificationCompat.Builder(this, CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_launcher)
                .setContentTitle(getString(R.string.audio_notification_title))
                .setContentText(text)
                .setContentIntent(openPendingIntent)
                .setCategory(NotificationCompat.CATEGORY_TRANSPORT)
                .setPriority(NotificationCompat.PRIORITY_LOW)
                .setOnlyAlertOnce(true)
                .setOngoing(true)
                .build();
    }

    private void showPlaybackNotification(File file) {
        if (!foreground) {
            Intent keepAlive = new Intent(this, AudioService.class)
                    .setAction(ACTION_KEEP_ALIVE);
            ContextCompat.startForegroundService(this, keepAlive);
        }
        Notification notification = buildNotification(file);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            startForeground(NOTIFICATION_ID, notification,
                    ServiceInfo.FOREGROUND_SERVICE_TYPE_MEDIA_PLAYBACK);
        } else {
            startForeground(NOTIFICATION_ID, notification);
        }
        foreground = true;
    }

    private void stopPlaybackForeground() {
        removeForegroundNotification();
        stopSelf();
    }

    private void removeForegroundNotification() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            stopForeground(STOP_FOREGROUND_REMOVE);
        } else {
            stopForeground(true);
        }
        foreground = false;
    }
}
