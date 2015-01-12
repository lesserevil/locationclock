package org.mass_hysteria.locationmonitor;

import android.app.Notification;
import android.service.notification.StatusBarNotification;
import android.util.Log;
import android.service.notification.NotificationListenerService;
import android.widget.TextView;

/**
 * Created by lesserevil on 1/9/2015.
 */
public class Life360NotificationListenerService extends NotificationListenerService {

    private final String TAG = this.getClass().getSimpleName();

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(this.getClass().getSimpleName(), "Service created");
    }

    @Override
    public void onNotificationPosted(StatusBarNotification sbn) {
        if (sbn.getPackageName().contains("com.life360.android")) {
            Log.i(TAG, "New: " + sbn.toString());
            Log.i(TAG, "Extras: " + sbn.getNotification().extras);

            Log.i(TAG, "Text: " + sbn.getNotification().extras.getCharSequence(Notification.EXTRA_TEXT));

            this.cancelNotification(sbn.getKey());

            Log.i(TAG, "Cancelled notification " + sbn.getKey());
        }

        super.onNotificationPosted(sbn);
    }

    @Override
    public void onNotificationRemoved(StatusBarNotification sbn) {
        super.onNotificationRemoved(sbn);
    }
}
