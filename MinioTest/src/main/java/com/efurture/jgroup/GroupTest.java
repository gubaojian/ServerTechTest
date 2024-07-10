package com.efurture.jgroup;

import org.jgroups.*;

import java.io.InputStream;

/**
 * zeromq: https://zeromq.org/download/
 * */
public class GroupTest {

    public static void main(String[] args) throws Exception {
        JChannel ch = null;
        try {
            InputStream in = GroupTest.class.getResourceAsStream("/tcp.xml");
            ch = new JChannel(in);
            ch.setReceiver(new Receiver() {
                @Override
                public void receive(Message msg) {
                    System.out.println("message received " + msg.<String>getObject());
                }

                @Override
                public void viewAccepted(View new_view) {
                    System.out.println("view received " + new_view);
                }
            });
            ch.connect("block_chain");
            while (true) {
                Message message = new BytesMessage();
                message.setObject("hello world " + ch.getAddressAsString());
                ch.send(message);
                Thread.sleep(100);
            }
        } finally {
            if (ch != null) {
                ch.close();
            }
        }
    }
}
