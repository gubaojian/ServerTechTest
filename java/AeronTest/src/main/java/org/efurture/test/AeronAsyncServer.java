package org.efurture.test;
import io.aeron.Aeron;
import io.aeron.Subscription;
import io.aeron.Publication;
import io.aeron.driver.MediaDriver;
import io.aeron.logbuffer.FragmentHandler;

public class AeronAsyncServer {
    private static final String SERVER_CHANNEL = "aeron:udp?endpoint=localhost:40123";
    private static final String CLIENT_CHANNEL = "aeron:udp?endpoint=localhost:40124";
    private static final int STREAM_ID = 10;
    private static final int MESSAGE_LENGTH = 1024;

    private static int messageCount = 0;

    public static void main(String[] args) throws Exception {

        MediaDriver.Context driverCtx = new MediaDriver.Context();
        driverCtx.ipcTermBufferLength(256*1024*1024);
        //driverCtx.mtuLength(32*1024);
        driverCtx.ipcMtuLength(32*1024);
        driverCtx.socketSndbufLength(2*1024*1024);
        driverCtx.socketRcvbufLength(2*1024*1024);
        driverCtx.dirDeleteOnStart(true);


        boolean run = true;
        MediaDriver driver = MediaDriver.launch(driverCtx);
        Aeron.Context ctx = new Aeron.Context();
        ctx = ctx.aeronDirectoryName(driver.aeronDirectoryName());
        Aeron aeron = Aeron.connect(ctx);
         try {
             Subscription subscription = aeron.addSubscription(SERVER_CHANNEL, STREAM_ID);
             // 创建发布发送响应
             Publication publication = aeron.addPublication(CLIENT_CHANNEL, STREAM_ID);
             // 消息处理器
             FragmentHandler handler = (buffer, offset, length, header) -> {
                 messageCount++;
                 if (messageCount % 10000 == 0) {
                     System.out.printf("Server received %d bytes%n  %d", length, messageCount);
                 }

                 // 直接复用接收的缓冲区发送响应（零拷贝）
                 //long result = publication.offer(buffer, offset, length);
                 //if (result > 0) {
                 //    System.out.println("Server sent response");
                // } else {
                 //    System.out.printf("Send failed: %d%n", result);
                 //}
             };

             System.out.println("Server started. Press Enter to exit...");
             while (true) {
                 subscription.poll(handler, 1000);
             }
         } catch (Exception e) {}
        aeron.close();
        driver.close();
    }
}
