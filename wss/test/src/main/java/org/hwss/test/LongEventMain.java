package org.hwss.test;

import com.lmax.disruptor.RingBuffer;
import com.lmax.disruptor.dsl.Disruptor;
import com.lmax.disruptor.util.DaemonThreadFactory;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicLong;

/**
 * send used 487ms
 * send used 420ms
 * */
public class LongEventMain
{
    public static void main(String[] args) throws Exception
    {
        final int size = 10000*300;
        int bufferSize = 4096;
        Disruptor<LongEvent> disruptor =
                new Disruptor<>(LongEvent::new, bufferSize, DaemonThreadFactory.INSTANCE);

        disruptor.handleEventsWith((event, sequence, endOfBatch) -> {
                 if (event.getValue() == size) {
                     System.out.println("receive end " + (System.currentTimeMillis()) + "ms");
                     System.out.println("disruptor Event: " + event + " " + Thread.currentThread().getName());
                 }
        });
        disruptor.handleEventsWith((event, sequence, endOfBatch) -> {
            if (event.getValue() == size) {
                System.out.println("receive end" + (System.currentTimeMillis()) + "ms");
                System.out.println("disruptor Event: " + event + " " + Thread.currentThread().getName());
            }
        });
        disruptor.start();


        RingBuffer<LongEvent> ringBuffer = disruptor.getRingBuffer();
        AtomicLong atomicLong = new AtomicLong();
        long start = System.currentTimeMillis();
        System.out.println("send start " + (start) + "ms");
        for (int i=0; i<size; i++)
        {
            final long eventLong = atomicLong.incrementAndGet();
            ringBuffer.publishEvent((event, sequence, buffer) -> event.set(eventLong));
        }
        long end = System.currentTimeMillis();
        System.out.println("send used " + (end - start) + "ms");
        System.out.println("long end value " + atomicLong.get());
        disruptor.shutdown();
    }
}