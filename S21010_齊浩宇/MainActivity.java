package com.example.myapplication;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import android.app.Notification;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.google.android.material.textfield.TextInputLayout;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;


public class MainActivity extends AppCompatActivity {

    static String MQTTHOST = "tcp://sunlab.kic.ac.jp";
    static String USERNAME = "AndroidTest";
    static String PASSWORD = "0000";
    String topicStr = "s21010/LockAlarm";

    MqttClient client;
    MqttConnectOptions options;
    String clientId;

    private NotificationManagerCompat notificationManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        clientId = MqttClient.generateClientId();

        notificationManager = NotificationManagerCompat.from(this);
        sendOnChannel("Welcome welcome!!");

        options = new MqttConnectOptions();
        options.setUserName(USERNAME);
        options.setPassword(PASSWORD.toCharArray());

        try {
            client = new MqttClient(MQTTHOST, clientId, new MemoryPersistence());
            client.connect(options);
            setSubscription();
        } catch (MqttException e) {
            e.printStackTrace();
        }

        client.setCallback(new MqttCallback() {
            @Override
            public void connectionLost(Throwable cause) {
                System.out.println("Connect lost");
                try {
                    client.connect(options);
                    client.subscribe(topicStr, 0);
                } catch (MqttException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void messageArrived(String topic, MqttMessage message) throws Exception {
                System.out.println(new String(message.getPayload()));
                sendOnChannel(new String(message.getPayload()));
            }

            @Override
            public void deliveryComplete(IMqttDeliveryToken token) {
                System.out.println("delivery Complete");
            }
        });

    }

    public void pubStop(View view) {
        String topic = topicStr;
        String message = "Got_it_its_ok";
        try {
            client.publish(topic, message.getBytes(), 0, false);
        } catch (MqttException e) {
            e.printStackTrace();
        }
        sendOnChannel("Message sended : " + message);
    }

    public void pubKill(View view) {
        String topic = topicStr;
        String message = "Maybe_its_broken_turn_off_all";
        try {
            client.publish(topic, message.getBytes(), 0, false);
        } catch (MqttException e) {
            e.printStackTrace();
        }
        sendOnChannel("Message sended : " + message);
    }

    public void bibi(View view) {
        String topic = topicStr;
        String message = "Im_here";
        try {
            client.publish(topic, message.getBytes(), 0, false);
        } catch (MqttException e) {
            e.printStackTrace();
        }
        sendOnChannel("Message sended : " + message);
    }

    private void setSubscription() {
        try {
            client.subscribe(topicStr, 0);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void sendOnChannel(String message) {
        Notification notification = new NotificationCompat.Builder(this, App.CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_one)
                .setContentTitle(topicStr)
                .setContentText(message)
                .setPriority(NotificationCompat.PRIORITY_HIGH)
                .setCategory(NotificationCompat.CATEGORY_MESSAGE)
                .build();

        notificationManager.notify(1, notification);
    }
}