package org.efurture.test;

import com.hivemq.client.mqtt.MqttClient;
import com.hivemq.client.mqtt.datatypes.MqttQos;
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient;

//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
public class Main {
    public static void main(String[] args) {
        Mqtt5BlockingClient client = MqttClient.builder()
                .useMqttVersion5()
                .identifier("JavaClient")
                .serverHost("localhost")
                .serverPort(1883)
                .buildBlocking();


        // 发布消息
        client.publishWith()
                .topic("pub/connId")
                .qos(MqttQos.AT_LEAST_ONCE)
                .payload("Hello, MQTT!".getBytes())
                .send();

        // 订阅主题
        client.subscribeWith().topicFilter("sub/connId")

        client.disconnect();
    }
}