package org.hwss.test.bench.improve;

import org.java_websocket.exceptions.InvalidDataException;
import org.java_websocket.framing.TextFrame;

/**
 * QuickTextFrame isValid() 方法需要遍历所有字符串，比较耗时，
 * client端发送时可以跳过这个验证。
 * 跳过后速度提升一， 速度稳定在500mb/s 打满网卡。不然都打不满网卡
 * start new turn total message 2000002
 * receive used 11974.2ms
 *
 * 单线程即可稳定在500mb，而且cpu不会飙升.
 *
 * */
public class ClientFastTextFrame extends TextFrame {
    @Override
    public void isValid() throws InvalidDataException {
        // skip valid， none need for client
        //super.isValid();
    }
}
