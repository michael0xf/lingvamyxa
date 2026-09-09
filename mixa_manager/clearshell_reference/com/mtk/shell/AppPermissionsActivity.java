package com.mtk.shell;

import android.Manifest;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlarmManager;
import android.app.NotificationManager;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PermissionInfo;
import android.content.pm.ResolveInfo;
import android.content.pm.ServiceInfo;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
import android.provider.Settings;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.app.NotificationManagerCompat;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * A deliberately conventional Android screen for inspecting another app's
 * manifest permissions and opening the corresponding system Settings pages.
 */
public class AppPermissionsActivity extends Activity {
    public static final String EXTRA_PACKAGE_NAME = "com.mtk.shell.extra.PACKAGE_NAME";

    private static final int STATUS_DENIED = 0;
    private static final int STATUS_GRANTED = 1;
    private static final int STATUS_UNKNOWN = 2;
    private static final int STATUS_INFO = 3;

    private static final int SETTINGS_DETAILS = 0;
    private static final int SETTINGS_HOME = 1;
    private static final int SETTINGS_ALL_FILES = 2;
    private static final int SETTINGS_INSTALL_APKS = 3;
    private static final int SETTINGS_NOTIFICATIONS = 4;
    private static final int SETTINGS_BACKGROUND_DATA = 5;
    private static final int SETTINGS_OVERLAY = 6;
    private static final int SETTINGS_WRITE_SETTINGS = 7;
    private static final int SETTINGS_USAGE_ACCESS = 8;
    private static final int SETTINGS_NOTIFICATION_POLICY = 9;
    private static final int SETTINGS_EXACT_ALARM = 10;
    private static final int SETTINGS_BATTERY = 11;
    private static final int SETTINGS_FULL_SCREEN_INTENT = 12;
    private static final int SETTINGS_APP_PERMISSIONS = 13;
    private static final int SETTINGS_NOTIFICATION_LISTENER = 14;

    private static final String PERMISSION_MANAGE_EXTERNAL_STORAGE =
            "android.permission.MANAGE_EXTERNAL_STORAGE";
    private static final String PERMISSION_REQUEST_INSTALL_PACKAGES =
            "android.permission.REQUEST_INSTALL_PACKAGES";
    private static final String PERMISSION_QUERY_ALL_PACKAGES =
            "android.permission.QUERY_ALL_PACKAGES";
    private static final String PERMISSION_FOREGROUND_MEDIA_PLAYBACK =
            "android.permission.FOREGROUND_SERVICE_MEDIA_PLAYBACK";
    private static final String PERMISSION_PACKAGE_USAGE_STATS =
            "android.permission.PACKAGE_USAGE_STATS";
    private static final String PERMISSION_ACCESS_NOTIFICATION_POLICY =
            "android.permission.ACCESS_NOTIFICATION_POLICY";
    private static final String PERMISSION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS =
            "android.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS";
    private static final String PERMISSION_SCHEDULE_EXACT_ALARM =
            "android.permission.SCHEDULE_EXACT_ALARM";
    private static final String PERMISSION_USE_EXACT_ALARM =
            "android.permission.USE_EXACT_ALARM";
    private static final String PERMISSION_USE_FULL_SCREEN_INTENT =
            "android.permission.USE_FULL_SCREEN_INTENT";

    private final ArrayList<PermissionEntry> entries = new ArrayList<>();
    private PermissionAdapter adapter;
    private String packageName;
    private ComponentName notificationListenerComponent;

    /** Returns true for a real user-actionable denial, excluding the optional Home role. */
    public static boolean hasDeniedPermissionsForSelf(Context context) {
        if (context == null) {
            return false;
        }
        PackageManager packageManager = context.getPackageManager();
        String ownPackage = context.getPackageName();
        try {
            if (BuildConfig.NOTIFICATION_LOG_ENABLED
                    && !isNotificationListenerAccessGranted(context,
                    new ComponentName(context, NotificationLogService.class))) {
                return true;
            }
            PackageInfo packageInfo = packageManager.getPackageInfo(ownPackage,
                    PackageManager.GET_PERMISSIONS);
            String[] permissions = packageInfo.requestedPermissions;
            int[] flags = packageInfo.requestedPermissionsFlags;
            if (permissions == null) {
                return false;
            }
            for (int i = 0; i < permissions.length; i++) {
                String permission = permissions[i];
                if (permission.endsWith(".DYNAMIC_RECEIVER_NOT_EXPORTED_PERMISSION")) {
                    continue;
                }
                if (isLegacyStoragePermissionUnavailable(permission)) {
                    continue;
                }
                PermissionInfo permissionInfo;
                try {
                    permissionInfo = packageManager.getPermissionInfo(permission, 0);
                } catch (PackageManager.NameNotFoundException unavailableOnThisAndroidVersion) {
                    continue;
                }

                if (PERMISSION_MANAGE_EXTERNAL_STORAGE.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R
                            && !Environment.isExternalStorageManager()) {
                        return true;
                    }
                    continue;
                }
                if (PERMISSION_REQUEST_INSTALL_PACKAGES.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O
                            && !packageManager.canRequestPackageInstalls()) {
                        return true;
                    }
                    continue;
                }
                if (Manifest.permission.SYSTEM_ALERT_WINDOW.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
                            && !Settings.canDrawOverlays(context)) {
                        return true;
                    }
                    continue;
                }
                if (Manifest.permission.WRITE_SETTINGS.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
                            && !Settings.System.canWrite(context)) {
                        return true;
                    }
                    continue;
                }
                if (PERMISSION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                        PowerManager powerManager =
                                (PowerManager) context.getSystemService(Context.POWER_SERVICE);
                        if (powerManager == null
                                || !powerManager.isIgnoringBatteryOptimizations(ownPackage)) {
                            return true;
                        }
                    }
                    continue;
                }
                if (PERMISSION_ACCESS_NOTIFICATION_POLICY.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                        NotificationManager notificationManager = (NotificationManager)
                                context.getSystemService(Context.NOTIFICATION_SERVICE);
                        if (notificationManager == null
                                || !notificationManager.isNotificationPolicyAccessGranted()) {
                            return true;
                        }
                    }
                    continue;
                }
                if (PERMISSION_SCHEDULE_EXACT_ALARM.equals(permission)
                        || PERMISSION_USE_EXACT_ALARM.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                        AlarmManager alarmManager =
                                (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
                        if (alarmManager == null || !alarmManager.canScheduleExactAlarms()) {
                            return true;
                        }
                    }
                    continue;
                }
                if (PERMISSION_USE_FULL_SCREEN_INTENT.equals(permission)) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
                        NotificationManager notificationManager = (NotificationManager)
                                context.getSystemService(Context.NOTIFICATION_SERVICE);
                        if (notificationManager == null
                                || !notificationManager.canUseFullScreenIntent()) {
                            return true;
                        }
                    }
                    continue;
                }
                // Usage access is an AppOp whose state is not represented by the
                // manifest grant flag. Keep it neutral, as the inspector does.
                if (PERMISSION_PACKAGE_USAGE_STATS.equals(permission)) {
                    continue;
                }

                // Signature/internal permissions cannot be granted by the user.
                // They may legitimately be reported as denied, but must not force
                // the inspector to open every time ClearShell starts.
                int protection = permissionInfo.protectionLevel
                        & PermissionInfo.PROTECTION_MASK_BASE;
                if (protection != PermissionInfo.PROTECTION_NORMAL
                        && protection != PermissionInfo.PROTECTION_DANGEROUS) {
                    continue;
                }

                boolean granted;
                if (flags != null && i < flags.length) {
                    granted = (flags[i] & PackageInfo.REQUESTED_PERMISSION_GRANTED) != 0;
                } else {
                    granted = packageManager.checkPermission(permission, ownPackage)
                            == PackageManager.PERMISSION_GRANTED;
                }
                if (!granted) {
                    return true;
                }
            }
            return false;
        } catch (Throwable ignored) {
            // Failure to inspect must never prevent ClearShell itself from starting.
            return false;
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        packageName = getIntent().getStringExtra(EXTRA_PACKAGE_NAME);
        if (packageName == null || packageName.trim().isEmpty()) {
            finish();
            return;
        }

        ListView listView = new ListView(this);
        adapter = new PermissionAdapter(this, entries);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener((parent, view, position, id) -> {
            PermissionEntry entry = entries.get(position);
            if (entry.clickable) {
                openSettings(entry.settings);
            }
        });
        setContentView(listView);

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setTitle("All permissions");
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        reloadPermissions();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void reloadPermissions() {
        PackageManager packageManager = getPackageManager();
        try {
            PackageInfo packageInfo = packageManager.getPackageInfo(packageName,
                    PackageManager.GET_PERMISSIONS | PackageManager.GET_SERVICES);
            CharSequence label = packageManager.getApplicationLabel(packageInfo.applicationInfo);
            ActionBar actionBar = getActionBar();
            if (actionBar != null) {
                actionBar.setSubtitle(label + " • " + packageName);
                Drawable icon = packageManager.getApplicationIcon(packageInfo.applicationInfo);
                actionBar.setIcon(icon);
            } else {
                setTitle(label + " — All permissions");
            }

            entries.clear();
            buildEntries(packageInfo, packageManager, entries);
            if (entries.isEmpty()) {
                entries.add(new PermissionEntry("No requested permissions", "",
                        STATUS_INFO, SETTINGS_DETAILS, false));
            }
            adapter.notifyDataSetChanged();
        } catch (PackageManager.NameNotFoundException e) {
            Toast.makeText(this, "Application is no longer installed", Toast.LENGTH_SHORT).show();
            finish();
        } catch (Throwable error) {
            Toast.makeText(this, "Cannot read permissions: " + error.getMessage(),
                    Toast.LENGTH_LONG).show();
        }
    }

    private void buildEntries(PackageInfo packageInfo, PackageManager packageManager,
                              ArrayList<PermissionEntry> result) {
        Map<String, Integer> statuses = requestedPermissionStatuses(packageInfo, packageManager);
        Set<String> consumed = new HashSet<>();

        if (isHomeCandidate(packageManager)) {
            result.add(new PermissionEntry("Home app", "Default Home application",
                    isDefaultHome(packageManager) ? STATUS_GRANTED : STATUS_DENIED,
                    SETTINGS_HOME, true));
        }

        addSpecialPermission(result, statuses, consumed,
                PERMISSION_MANAGE_EXTERNAL_STORAGE, "All files access",
                SETTINGS_ALL_FILES, specialAllFilesStatus());
        addSpecialPermission(result, statuses, consumed,
                PERMISSION_REQUEST_INSTALL_PACKAGES, "Install APK",
                SETTINGS_INSTALL_APKS, specialInstallApksStatus());
        addPermissionGroup(result, statuses, consumed, "Camera", SETTINGS_APP_PERMISSIONS,
                Manifest.permission.CAMERA);
        addPermissionGroup(result, statuses, consumed, "Contacts", SETTINGS_APP_PERMISSIONS,
                Manifest.permission.READ_CONTACTS,
                Manifest.permission.WRITE_CONTACTS,
                Manifest.permission.GET_ACCOUNTS);
        addPermissionGroup(result, statuses, consumed, "Phone", SETTINGS_APP_PERMISSIONS,
                Manifest.permission.CALL_PHONE,
                Manifest.permission.READ_PHONE_STATE,
                Manifest.permission.READ_PHONE_NUMBERS,
                Manifest.permission.ANSWER_PHONE_CALLS,
                Manifest.permission.READ_CALL_LOG,
                Manifest.permission.WRITE_CALL_LOG,
                Manifest.permission.ADD_VOICEMAIL,
                Manifest.permission.USE_SIP,
                Manifest.permission.PROCESS_OUTGOING_CALLS);
        addPermissionGroup(result, statuses, consumed, "Notifications",
                SETTINGS_NOTIFICATIONS, Manifest.permission.POST_NOTIFICATIONS);
        notificationListenerComponent = findNotificationListenerComponent(packageInfo);
        if (notificationListenerComponent != null) {
            // Android only allows the exact component-level check for the
            // calling app. Keep another app's special-access state neutral.
            int status = isOwnPackage()
                    ? (isNotificationListenerAccessGranted(this,
                    notificationListenerComponent) ? STATUS_GRANTED : STATUS_DENIED)
                    : STATUS_UNKNOWN;
            result.add(new PermissionEntry("Notification access",
                    "Read incoming notifications",
                    status, SETTINGS_NOTIFICATION_LISTENER, true));
        }
        addPermissionGroup(result, statuses, consumed, "Network",
                SETTINGS_BACKGROUND_DATA, Manifest.permission.INTERNET);
        addPermissionGroup(result, statuses, consumed, "Apps visibility",
                SETTINGS_DETAILS, PERMISSION_QUERY_ALL_PACKAGES);
        if (statuses.containsKey(PERMISSION_FOREGROUND_MEDIA_PLAYBACK)) {
            addPermissionGroup(result, statuses, consumed, "Background audio",
                    SETTINGS_DETAILS,
                    PERMISSION_FOREGROUND_MEDIA_PLAYBACK,
                    Manifest.permission.FOREGROUND_SERVICE);
        }

        ArrayList<PermissionEntry> remaining = new ArrayList<>();
        for (Map.Entry<String, Integer> permission : statuses.entrySet()) {
            if (consumed.contains(permission.getKey())) {
                continue;
            }
            int settings = settingsForPermission(permission.getKey(), packageManager);
            int status = permission.getValue();
            status = specialPermissionStatus(permission.getKey(), status);
            remaining.add(new PermissionEntry(
                    permissionLabel(packageManager, permission.getKey()),
                    permission.getKey(), status, settings, true));
        }
        Collections.sort(remaining, new Comparator<PermissionEntry>() {
            @Override
            public int compare(PermissionEntry first, PermissionEntry second) {
                return first.title.compareToIgnoreCase(second.title);
            }
        });
        result.addAll(remaining);
    }

    private Map<String, Integer> requestedPermissionStatuses(PackageInfo packageInfo,
                                                              PackageManager packageManager) {
        Map<String, Integer> result = new HashMap<>();
        String[] permissions = packageInfo.requestedPermissions;
        int[] flags = packageInfo.requestedPermissionsFlags;
        if (permissions == null) {
            return result;
        }
        for (int i = 0; i < permissions.length; i++) {
            String permission = permissions[i];
            // AndroidX adds this internal signature permission while merging the
            // manifest. It protects framework receiver plumbing and is not an
            // app capability that a user can inspect or change.
            if (permission.endsWith(".DYNAMIC_RECEIVER_NOT_EXPORTED_PERMISSION")) {
                continue;
            }
            if (isLegacyStoragePermissionUnavailable(permission)) {
                continue;
            }
            try {
                packageManager.getPermissionInfo(permission, 0);
            } catch (PackageManager.NameNotFoundException unavailableOnThisAndroidVersion) {
                result.put(permission, STATUS_UNKNOWN);
                continue;
            }
            boolean granted;
            if (flags != null && i < flags.length) {
                granted = (flags[i] & PackageInfo.REQUESTED_PERMISSION_GRANTED) != 0;
            } else {
                granted = packageManager.checkPermission(permission, packageName)
                        == PackageManager.PERMISSION_GRANTED;
            }
            result.put(permission, granted ? STATUS_GRANTED : STATUS_DENIED);
        }
        return result;
    }

    private static boolean isLegacyStoragePermissionUnavailable(String permission) {
        return (Manifest.permission.READ_EXTERNAL_STORAGE.equals(permission)
                && Build.VERSION.SDK_INT > Build.VERSION_CODES.S_V2)
                || (Manifest.permission.WRITE_EXTERNAL_STORAGE.equals(permission)
                && Build.VERSION.SDK_INT > Build.VERSION_CODES.Q);
    }

    private void addSpecialPermission(ArrayList<PermissionEntry> result,
                                      Map<String, Integer> statuses,
                                      Set<String> consumed,
                                      String permission, String title, int settings,
                                      int exactStatus) {
        if (!statuses.containsKey(permission)) {
            return;
        }
        consumed.add(permission);
        int status = isOwnPackage() ? exactStatus : STATUS_UNKNOWN;
        String detail = status == STATUS_UNKNOWN
                ? "Special access • tap to check in Android Settings"
                : permission;
        result.add(new PermissionEntry(title, detail, status, settings, true));
    }

    private void addPermissionGroup(ArrayList<PermissionEntry> result,
                                    Map<String, Integer> statuses,
                                    Set<String> consumed,
                                    String title, int settings, String... permissions) {
        ArrayList<String> requested = new ArrayList<>();
        int combinedStatus = STATUS_GRANTED;
        for (String permission : permissions) {
            Integer status = statuses.get(permission);
            if (status != null) {
                requested.add(permission);
                consumed.add(permission);
                if (status == STATUS_DENIED) {
                    combinedStatus = STATUS_DENIED;
                } else if (status == STATUS_UNKNOWN && combinedStatus == STATUS_GRANTED) {
                    combinedStatus = STATUS_UNKNOWN;
                }
            }
        }
        if (requested.isEmpty()) {
            return;
        }
        result.add(new PermissionEntry(title, joinPermissions(requested),
                combinedStatus, settings, true));
    }

    private String joinPermissions(List<String> permissions) {
        StringBuilder result = new StringBuilder();
        for (String permission : permissions) {
            if (result.length() > 0) {
                result.append(" • ");
            }
            int dot = permission.lastIndexOf('.');
            result.append(dot >= 0 ? permission.substring(dot + 1) : permission);
        }
        return result.toString();
    }

    private String permissionLabel(PackageManager packageManager, String permission) {
        String known = knownPermissionLabel(permission);
        if (known != null) {
            return known;
        }
        try {
            PermissionInfo permissionInfo = packageManager.getPermissionInfo(permission, 0);
            CharSequence label = permissionInfo.loadLabel(packageManager);
            if (label != null && label.length() > 0) {
                return label.toString();
            }
        } catch (PackageManager.NameNotFoundException ignored) {
        }
        int dot = permission.lastIndexOf('.');
        return dot >= 0 ? permission.substring(dot + 1) : permission;
    }

    private String knownPermissionLabel(String permission) {
        if (Manifest.permission.SYSTEM_ALERT_WINDOW.equals(permission)) {
            return "Display over other apps";
        }
        if (Manifest.permission.WRITE_SETTINGS.equals(permission)) {
            return "Modify system settings";
        }
        if (PERMISSION_PACKAGE_USAGE_STATS.equals(permission)) {
            return "Usage access";
        }
        if (PERMISSION_ACCESS_NOTIFICATION_POLICY.equals(permission)) {
            return "Do Not Disturb access";
        }
        if (PERMISSION_SCHEDULE_EXACT_ALARM.equals(permission)
                || PERMISSION_USE_EXACT_ALARM.equals(permission)) {
            return "Alarms & reminders";
        }
        if (PERMISSION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS.equals(permission)) {
            return "Battery optimization";
        }
        if (PERMISSION_USE_FULL_SCREEN_INTENT.equals(permission)) {
            return "Full-screen notifications";
        }
        return null;
    }

    private int settingsForPermission(String permission, PackageManager packageManager) {
        if (Manifest.permission.SYSTEM_ALERT_WINDOW.equals(permission)) {
            return SETTINGS_OVERLAY;
        }
        if (Manifest.permission.WRITE_SETTINGS.equals(permission)) {
            return SETTINGS_WRITE_SETTINGS;
        }
        if (PERMISSION_PACKAGE_USAGE_STATS.equals(permission)) {
            return SETTINGS_USAGE_ACCESS;
        }
        if (PERMISSION_ACCESS_NOTIFICATION_POLICY.equals(permission)) {
            return SETTINGS_NOTIFICATION_POLICY;
        }
        if (PERMISSION_SCHEDULE_EXACT_ALARM.equals(permission)
                || PERMISSION_USE_EXACT_ALARM.equals(permission)) {
            return SETTINGS_EXACT_ALARM;
        }
        if (PERMISSION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS.equals(permission)) {
            return SETTINGS_BATTERY;
        }
        if (PERMISSION_USE_FULL_SCREEN_INTENT.equals(permission)) {
            return SETTINGS_FULL_SCREEN_INTENT;
        }
        try {
            PermissionInfo permissionInfo = packageManager.getPermissionInfo(permission, 0);
            if ((permissionInfo.protectionLevel & PermissionInfo.PROTECTION_MASK_BASE)
                    == PermissionInfo.PROTECTION_DANGEROUS) {
                return SETTINGS_APP_PERMISSIONS;
            }
        } catch (PackageManager.NameNotFoundException ignored) {
        }
        return SETTINGS_DETAILS;
    }

    private boolean isSpecialPermission(String permission) {
        return Manifest.permission.SYSTEM_ALERT_WINDOW.equals(permission)
                || Manifest.permission.WRITE_SETTINGS.equals(permission)
                || PERMISSION_PACKAGE_USAGE_STATS.equals(permission)
                || PERMISSION_ACCESS_NOTIFICATION_POLICY.equals(permission)
                || PERMISSION_SCHEDULE_EXACT_ALARM.equals(permission)
                || PERMISSION_USE_EXACT_ALARM.equals(permission)
                || PERMISSION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS.equals(permission)
                || PERMISSION_USE_FULL_SCREEN_INTENT.equals(permission);
    }

    private int specialPermissionStatus(String permission, int manifestStatus) {
        if (PERMISSION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS.equals(permission)
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            PowerManager powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
            return powerManager != null && powerManager.isIgnoringBatteryOptimizations(packageName)
                    ? STATUS_GRANTED : STATUS_DENIED;
        }
        if (isSpecialPermission(permission) && !isOwnPackage()) {
            return STATUS_UNKNOWN;
        }
        if (Manifest.permission.SYSTEM_ALERT_WINDOW.equals(permission)
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return Settings.canDrawOverlays(this) ? STATUS_GRANTED : STATUS_DENIED;
        }
        if (Manifest.permission.WRITE_SETTINGS.equals(permission)
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            return Settings.System.canWrite(this) ? STATUS_GRANTED : STATUS_DENIED;
        }
        if ((PERMISSION_SCHEDULE_EXACT_ALARM.equals(permission)
                || PERMISSION_USE_EXACT_ALARM.equals(permission))
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            AlarmManager alarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
            return alarmManager != null && alarmManager.canScheduleExactAlarms()
                    ? STATUS_GRANTED : STATUS_DENIED;
        }
        if (PERMISSION_ACCESS_NOTIFICATION_POLICY.equals(permission)
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            NotificationManager notificationManager =
                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            return notificationManager != null
                    && notificationManager.isNotificationPolicyAccessGranted()
                    ? STATUS_GRANTED : STATUS_DENIED;
        }
        if (PERMISSION_USE_FULL_SCREEN_INTENT.equals(permission)
                && Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
            NotificationManager notificationManager =
                    (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            return notificationManager != null && notificationManager.canUseFullScreenIntent()
                    ? STATUS_GRANTED : STATUS_DENIED;
        }
        return isSpecialPermission(permission) ? STATUS_UNKNOWN : manifestStatus;
    }

    private int specialAllFilesStatus() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager() ? STATUS_GRANTED : STATUS_DENIED;
        }
        return getPackageManager().checkPermission(Manifest.permission.READ_EXTERNAL_STORAGE,
                packageName) == PackageManager.PERMISSION_GRANTED
                ? STATUS_GRANTED : STATUS_DENIED;
    }

    private int specialInstallApksStatus() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            return getPackageManager().canRequestPackageInstalls()
                    ? STATUS_GRANTED : STATUS_DENIED;
        }
        return STATUS_UNKNOWN;
    }

    private boolean isOwnPackage() {
        return getPackageName().equals(packageName);
    }

    private static ComponentName findNotificationListenerComponent(PackageInfo packageInfo) {
        if (packageInfo == null || packageInfo.services == null) {
            return null;
        }
        for (ServiceInfo service : packageInfo.services) {
            if (service != null
                    && Manifest.permission.BIND_NOTIFICATION_LISTENER_SERVICE
                    .equals(service.permission)) {
                return new ComponentName(service.packageName, service.name);
            }
        }
        return null;
    }

    private static boolean isNotificationListenerAccessGranted(Context context,
                                                                ComponentName component) {
        if (context == null || component == null) {
            return false;
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1) {
            NotificationManager notificationManager = (NotificationManager)
                    context.getSystemService(Context.NOTIFICATION_SERVICE);
            return notificationManager != null
                    && notificationManager.isNotificationListenerAccessGranted(component);
        }
        return NotificationManagerCompat.getEnabledListenerPackages(context)
                .contains(component.getPackageName());
    }

    private boolean isHomeCandidate(PackageManager packageManager) {
        return isHomeCandidate(packageManager, packageName);
    }

    private static boolean isHomeCandidate(PackageManager packageManager,
                                           String inspectedPackage) {
        Intent home = new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_HOME);
        home.setPackage(inspectedPackage);
        List<ResolveInfo> candidates = packageManager.queryIntentActivities(home,
                PackageManager.MATCH_DEFAULT_ONLY);
        return candidates != null && !candidates.isEmpty();
    }

    private boolean isDefaultHome(PackageManager packageManager) {
        return isDefaultHome(packageManager, packageName);
    }

    private static boolean isDefaultHome(PackageManager packageManager,
                                         String inspectedPackage) {
        Intent home = new Intent(Intent.ACTION_MAIN).addCategory(Intent.CATEGORY_HOME);
        ResolveInfo resolved = packageManager.resolveActivity(home, PackageManager.MATCH_DEFAULT_ONLY);
        return resolved != null && resolved.activityInfo != null
                && inspectedPackage.equals(resolved.activityInfo.packageName);
    }

    private void openSettings(int settings) {
        Intent intent = null;
        switch (settings) {
            case SETTINGS_HOME:
                intent = new Intent(Settings.ACTION_HOME_SETTINGS);
                break;
            case SETTINGS_ALL_FILES:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                    intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                            packageUri());
                    if (tryStart(intent)) {
                        return;
                    }
                    if (tryStart(new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION))) {
                        return;
                    }
                }
                break;
            case SETTINGS_INSTALL_APKS:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    intent = new Intent(Settings.ACTION_MANAGE_UNKNOWN_APP_SOURCES, packageUri());
                }
                break;
            case SETTINGS_NOTIFICATIONS:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    intent = new Intent(Settings.ACTION_APP_NOTIFICATION_SETTINGS)
                            .putExtra(Settings.EXTRA_APP_PACKAGE, packageName);
                } else {
                    intent = new Intent("android.settings.APP_NOTIFICATION_SETTINGS")
                            .putExtra("app_package", packageName);
                    try {
                        intent.putExtra("app_uid",
                                getPackageManager().getApplicationInfo(packageName, 0).uid);
                    } catch (PackageManager.NameNotFoundException ignored) {
                    }
                }
                break;
            case SETTINGS_BACKGROUND_DATA:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                    intent = new Intent(Settings.ACTION_IGNORE_BACKGROUND_DATA_RESTRICTIONS_SETTINGS,
                            packageUri());
                }
                break;
            case SETTINGS_OVERLAY:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION, packageUri());
                }
                break;
            case SETTINGS_WRITE_SETTINGS:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    intent = new Intent(Settings.ACTION_MANAGE_WRITE_SETTINGS, packageUri());
                }
                break;
            case SETTINGS_USAGE_ACCESS:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                    intent = new Intent(Settings.ACTION_USAGE_ACCESS_SETTINGS);
                }
                break;
            case SETTINGS_NOTIFICATION_POLICY:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    intent = new Intent(Settings.ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS);
                }
                break;
            case SETTINGS_EXACT_ALARM:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    intent = new Intent(Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM, packageUri());
                }
                break;
            case SETTINGS_BATTERY:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                    intent = new Intent(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS);
                }
                break;
            case SETTINGS_FULL_SCREEN_INTENT:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
                    intent = new Intent(Settings.ACTION_MANAGE_APP_USE_FULL_SCREEN_INTENT,
                            packageUri());
                }
                break;
            case SETTINGS_APP_PERMISSIONS:
                intent = new Intent("android.intent.action.MANAGE_APP_PERMISSIONS")
                        .putExtra(Intent.EXTRA_PACKAGE_NAME, packageName);
                break;
            case SETTINGS_NOTIFICATION_LISTENER:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R
                        && notificationListenerComponent != null) {
                    intent = new Intent(Settings.ACTION_NOTIFICATION_LISTENER_DETAIL_SETTINGS)
                            .putExtra(Settings.EXTRA_NOTIFICATION_LISTENER_COMPONENT_NAME,
                                    notificationListenerComponent.flattenToString());
                    if (tryStart(intent)) {
                        return;
                    }
                }
                intent = new Intent(Settings.ACTION_NOTIFICATION_LISTENER_SETTINGS);
                break;
            default:
                break;
        }
        if (intent == null || !tryStart(intent)) {
            openApplicationDetails();
        }
    }

    private Uri packageUri() {
        return Uri.fromParts("package", packageName, null);
    }

    private boolean tryStart(Intent intent) {
        try {
            if (intent.resolveActivity(getPackageManager()) == null) {
                return false;
            }
            startActivity(intent);
            return true;
        } catch (ActivityNotFoundException | SecurityException error) {
            return false;
        }
    }

    private void openApplicationDetails() {
        Intent details = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS, packageUri());
        if (!tryStart(details)) {
            Toast.makeText(this, "Settings are not available on this device",
                    Toast.LENGTH_SHORT).show();
        }
    }

    private static class PermissionEntry {
        final String title;
        final String detail;
        final int status;
        final int settings;
        final boolean clickable;

        PermissionEntry(String title, String detail, int status, int settings,
                        boolean clickable) {
            this.title = title;
            this.detail = detail;
            this.status = status;
            this.settings = settings;
            this.clickable = clickable;
        }

        String displayTitle() {
            switch (status) {
                case STATUS_GRANTED:
                    return "✓ " + title;
                case STATUS_DENIED:
                    return "✗ " + title;
                case STATUS_UNKNOWN:
                    return "→ " + title;
                default:
                    return title;
            }
        }
    }

    private static class PermissionAdapter extends ArrayAdapter<PermissionEntry> {
        PermissionAdapter(Context context, List<PermissionEntry> permissions) {
            super(context, android.R.layout.simple_list_item_2, android.R.id.text1, permissions);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = super.getView(position, convertView, parent);
            PermissionEntry entry = getItem(position);
            TextView title = view.findViewById(android.R.id.text1);
            TextView detail = view.findViewById(android.R.id.text2);
            if (entry != null) {
                title.setText(entry.displayTitle());
                detail.setText(entry.detail);
                detail.setVisibility(entry.detail.isEmpty() ? View.GONE : View.VISIBLE);
                view.setEnabled(entry.clickable);
            }
            return view;
        }
    }
}
