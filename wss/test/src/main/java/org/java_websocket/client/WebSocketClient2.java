package org.java_websocket.client;


import org.java_websocket.AbstractWebSocket;
import org.java_websocket.WebSocket;
import org.java_websocket.WebSocketImpl;
import org.java_websocket.drafts.Draft;
import org.java_websocket.drafts.Draft_6455;
import org.java_websocket.enums.Opcode;
import org.java_websocket.enums.ReadyState;
import org.java_websocket.exceptions.InvalidHandshakeException;
import org.java_websocket.framing.Framedata;
import org.java_websocket.handshake.HandshakeImpl1Client;
import org.java_websocket.handshake.Handshakedata;
import org.java_websocket.handshake.ServerHandshake;
import org.java_websocket.protocols.IProtocol;

import javax.net.SocketFactory;
import javax.net.ssl.*;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.InvocationTargetException;
import java.net.*;
import java.nio.ByteBuffer;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.util.*;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public abstract class WebSocketClient2 extends AbstractWebSocket implements Runnable, WebSocket {
    protected URI uri;
    private WebSocketImpl engine;
    private Socket socket;
    private SocketFactory socketFactory;
    private OutputStream ostream;
    private Proxy proxy;
    private Thread writeThread;
    private Thread connectReadThread;
    private Draft draft;
    private Map<String, String> headers;
    private CountDownLatch connectLatch;
    private CountDownLatch closeLatch;
    private int connectTimeout;
    private DnsResolver dnsResolver;

    public WebSocketClient2(URI serverUri) {
        this(serverUri, (Draft)(new Draft_6455()));
    }

    public WebSocketClient2(URI serverUri, Draft protocolDraft) {
        this(serverUri, protocolDraft, (Map)null, 0);
    }

    public WebSocketClient2(URI serverUri, Map<String, String> httpHeaders) {
        this(serverUri, new Draft_6455(), httpHeaders);
    }

    public WebSocketClient2(URI serverUri, Draft protocolDraft, Map<String, String> httpHeaders) {
        this(serverUri, protocolDraft, httpHeaders, 0);
    }

    public WebSocketClient2(URI serverUri, Draft protocolDraft, Map<String, String> httpHeaders, int connectTimeout) {
        this.uri = null;
        this.engine = null;
        this.socket = null;
        this.socketFactory = null;
        this.proxy = Proxy.NO_PROXY;
        this.connectLatch = new CountDownLatch(1);
        this.closeLatch = new CountDownLatch(1);
        this.connectTimeout = 0;
        this.dnsResolver = null;
        if (serverUri == null) {
            throw new IllegalArgumentException();
        } else if (protocolDraft == null) {
            throw new IllegalArgumentException("null as draft is permitted for `WebSocketServer` only!");
        } else {
            this.uri = serverUri;
            this.draft = protocolDraft;
            this.dnsResolver = new DnsResolver() {
                public InetAddress resolve(URI uri) throws UnknownHostException {
                    return InetAddress.getByName(uri.getHost());
                }
            };
            if (httpHeaders != null) {
                this.headers = new TreeMap(String.CASE_INSENSITIVE_ORDER);
                this.headers.putAll(httpHeaders);
            }

            this.connectTimeout = connectTimeout;
            this.setTcpNoDelay(false);
            this.setReuseAddr(false);
            this.engine = new WebSocketImpl(this, protocolDraft);
        }
    }

    public URI getURI() {
        return this.uri;
    }

    public Draft getDraft() {
        return this.draft;
    }

    public Socket getSocket() {
        return this.socket;
    }

    public void addHeader(String key, String value) {
        if (this.headers == null) {
            this.headers = new TreeMap(String.CASE_INSENSITIVE_ORDER);
        }

        this.headers.put(key, value);
    }

    public String removeHeader(String key) {
        return this.headers == null ? null : (String)this.headers.remove(key);
    }

    public void clearHeaders() {
        this.headers = null;
    }

    public void setDnsResolver(DnsResolver dnsResolver) {
        this.dnsResolver = dnsResolver;
    }

    public void reconnect() {
        this.reset();
        this.connect();
    }

    public boolean reconnectBlocking() throws InterruptedException {
        this.reset();
        return this.connectBlocking();
    }

    private void reset() {
        Thread current = Thread.currentThread();
        if (current != this.writeThread && current != this.connectReadThread) {
            try {
                if (this.engine.getReadyState() == ReadyState.NOT_YET_CONNECTED && this.socket != null) {
                    this.socket.close();
                }

                this.closeBlocking();
                if (this.writeThread != null) {
                    this.writeThread.interrupt();
                    this.writeThread.join();
                    this.writeThread = null;
                }

                if (this.connectReadThread != null) {
                    this.connectReadThread.interrupt();
                    this.connectReadThread.join();
                    this.connectReadThread = null;
                }

                this.draft.reset();
                if (this.socket != null) {
                    this.socket.close();
                    this.socket = null;
                }
            } catch (Exception var3) {
                Exception e = var3;
                this.onError(e);
                this.engine.closeConnection(1006, e.getMessage());
                return;
            }

            this.connectLatch = new CountDownLatch(1);
            this.closeLatch = new CountDownLatch(1);
            this.engine = new WebSocketImpl(this, this.draft);
        } else {
            throw new IllegalStateException("You cannot initialize a reconnect out of the websocket thread. Use reconnect in another thread to ensure a successful cleanup.");
        }
    }

    public void connect() {
        if (this.connectReadThread != null) {
            throw new IllegalStateException("WebSocketClient objects are not reuseable");
        } else {
            this.connectReadThread = new Thread(this);
            this.connectReadThread.setDaemon(this.isDaemon());
            this.connectReadThread.setName("WebSocketConnectReadThread-" + this.connectReadThread.getId());
            this.connectReadThread.start();
        }
    }

    public boolean connectBlocking() throws InterruptedException {
        this.connect();
        this.connectLatch.await();
        return this.engine.isOpen();
    }

    public boolean connectBlocking(long timeout, TimeUnit timeUnit) throws InterruptedException {
        this.connect();
        boolean connected = this.connectLatch.await(timeout, timeUnit);
        if (!connected) {
            this.reset();
        }

        return connected && this.engine.isOpen();
    }

    public void close() {
        if (this.writeThread != null) {
            this.engine.close(1000);
        }

    }

    public void closeBlocking() throws InterruptedException {
        this.close();
        this.closeLatch.await();
    }

    public void send(String text) {
        this.engine.send(text);
    }

    public void send(byte[] data) {
        this.engine.send(data);
    }

    public <T> T getAttachment() {
        return this.engine.getAttachment();
    }

    public <T> void setAttachment(T attachment) {
        this.engine.setAttachment(attachment);
    }

    protected Collection<WebSocket> getConnections() {
        return Collections.singletonList(this.engine);
    }

    public void sendPing() {
        this.engine.sendPing();
    }

    public void run() {
        InputStream istream;
        try {
            boolean upgradeSocketToSSLSocket = this.prepareSocket();
            this.socket.setTcpNoDelay(this.isTcpNoDelay());
            this.socket.setReuseAddress(this.isReuseAddr());
            int receiveBufferSize = this.getReceiveBufferSize();
            if (receiveBufferSize > 0) {
                this.socket.setReceiveBufferSize(receiveBufferSize);
            }

            if (!this.socket.isConnected()) {
                InetSocketAddress addr = this.dnsResolver == null ? InetSocketAddress.createUnresolved(this.uri.getHost(), this.getPort()) : new InetSocketAddress(this.dnsResolver.resolve(this.uri), this.getPort());
                this.socket.connect(addr, this.connectTimeout);
            }

            if (upgradeSocketToSSLSocket && "wss".equals(this.uri.getScheme())) {
                this.upgradeSocketToSSL();
            }

            if (this.socket instanceof SSLSocket) {
                SSLSocket sslSocket = (SSLSocket)this.socket;
                SSLParameters sslParameters = sslSocket.getSSLParameters();
                this.onSetSSLParameters(sslParameters);
                sslSocket.setSSLParameters(sslParameters);
            }

            istream = this.socket.getInputStream();
            this.ostream = this.socket.getOutputStream();
            this.sendHandshake();
        } catch (Exception var9) {
            Exception e = var9;
            this.onWebsocketError(this.engine, e);
            this.engine.closeConnection(-1, e.getMessage());
            return;
        } catch (InternalError var10) {
            InternalError e = var10;
            if (e.getCause() instanceof InvocationTargetException && e.getCause().getCause() instanceof IOException) {
                IOException cause = (IOException)e.getCause().getCause();
                this.onWebsocketError(this.engine, cause);
                this.engine.closeConnection(-1, cause.getMessage());
                return;
            }

            throw e;
        }

        if (this.writeThread != null) {
            this.writeThread.interrupt();

            try {
                this.writeThread.join();
            } catch (InterruptedException var6) {
            }
        }

        this.writeThread = new Thread(new WebsocketWriteThread(this));
        this.writeThread.setDaemon(this.isDaemon());
        this.writeThread.start();
        int receiveBufferSize = this.getReceiveBufferSize();
        byte[] rawbuffer = new byte[receiveBufferSize > 0 ? receiveBufferSize : DEFAULT_READ_BUFFER_SIZE];

        try {
            int readBytes;
            while(!this.isClosing() && !this.isClosed() && (readBytes = istream.read(rawbuffer)) != -1) {
                this.engine.decode(ByteBuffer.wrap(rawbuffer, 0, readBytes));
            }

            this.engine.eot();
        } catch (IOException var7) {
            IOException e = var7;
            this.handleIOException(e);
        } catch (RuntimeException var8) {
            RuntimeException e = var8;
            this.onError(e);
            this.engine.closeConnection(1006, e.getMessage());
        }

    }

    private void upgradeSocketToSSL() throws NoSuchAlgorithmException, KeyManagementException, IOException {
        SSLSocketFactory factory;
        if (this.socketFactory instanceof SSLSocketFactory) {
            factory = (SSLSocketFactory)this.socketFactory;
        } else {
            factory = (SSLSocketFactory)SSLSocketFactory.getDefault();
        }

        this.socket = factory.createSocket(this.socket, this.uri.getHost(), this.getPort(), true);
    }

    private boolean prepareSocket() throws IOException {
        boolean upgradeSocketToSSLSocket = false;
        if (this.proxy != Proxy.NO_PROXY) {
            this.socket = new Socket(this.proxy);
            upgradeSocketToSSLSocket = true;
        } else if (this.socketFactory != null) {
            this.socket = this.socketFactory.createSocket();
        } else if (this.socket == null) {
            this.socket = new Socket(this.proxy);
            upgradeSocketToSSLSocket = true;
        } else if (this.socket.isClosed()) {
            throw new IOException();
        }

        return upgradeSocketToSSLSocket;
    }

    protected void onSetSSLParameters(SSLParameters sslParameters) {
        sslParameters.setEndpointIdentificationAlgorithm("HTTPS");
    }

    private int getPort() {
        int port = this.uri.getPort();
        String scheme = this.uri.getScheme();
        if ("wss".equals(scheme)) {
            return port == -1 ? 443 : port;
        } else if ("ws".equals(scheme)) {
            return port == -1 ? 80 : port;
        } else {
            throw new IllegalArgumentException("unknown scheme: " + scheme);
        }
    }

    private void sendHandshake() throws InvalidHandshakeException {
        String part1 = this.uri.getRawPath();
        String part2 = this.uri.getRawQuery();
        String path;
        if (part1 != null && part1.length() != 0) {
            path = part1;
        } else {
            path = "/";
        }

        if (part2 != null) {
            path = path + '?' + part2;
        }

        int port = this.getPort();
        String host = this.uri.getHost() + (port != 80 && port != 443 ? ":" + port : "");
        HandshakeImpl1Client handshake = new HandshakeImpl1Client();
        handshake.setResourceDescriptor(path);
        handshake.put("Host", host);
        if (this.headers != null) {
            Iterator var7 = this.headers.entrySet().iterator();

            while(var7.hasNext()) {
                Map.Entry<String, String> kv = (Map.Entry)var7.next();
                handshake.put((String)kv.getKey(), (String)kv.getValue());
            }
        }

        this.engine.startHandshake(handshake);
    }

    public ReadyState getReadyState() {
        return this.engine.getReadyState();
    }

    public final void onWebsocketMessage(WebSocket conn, String message) {
        this.onMessage(message);
    }

    public final void onWebsocketMessage(WebSocket conn, ByteBuffer blob) {
        this.onMessage(blob);
    }

    public final void onWebsocketOpen(WebSocket conn, Handshakedata handshake) {
        this.startConnectionLostTimer();
        this.onOpen((ServerHandshake)handshake);
        this.connectLatch.countDown();
    }

    public final void onWebsocketClose(WebSocket conn, int code, String reason, boolean remote) {
        this.stopConnectionLostTimer();
        if (this.writeThread != null) {
            this.writeThread.interrupt();
        }

        this.onClose(code, reason, remote);
        this.connectLatch.countDown();
        this.closeLatch.countDown();
    }

    public final void onWebsocketError(WebSocket conn, Exception ex) {
        this.onError(ex);
    }

    public final void onWriteDemand(WebSocket conn) {
    }

    public void onWebsocketCloseInitiated(WebSocket conn, int code, String reason) {
        this.onCloseInitiated(code, reason);
    }

    public void onWebsocketClosing(WebSocket conn, int code, String reason, boolean remote) {
        this.onClosing(code, reason, remote);
    }

    public void onCloseInitiated(int code, String reason) {
    }

    public void onClosing(int code, String reason, boolean remote) {
    }

    public WebSocket getConnection() {
        return this.engine;
    }

    public InetSocketAddress getLocalSocketAddress(WebSocket conn) {
        return this.socket != null ? (InetSocketAddress)this.socket.getLocalSocketAddress() : null;
    }

    public InetSocketAddress getRemoteSocketAddress(WebSocket conn) {
        return this.socket != null ? (InetSocketAddress)this.socket.getRemoteSocketAddress() : null;
    }

    public abstract void onOpen(ServerHandshake var1);

    public abstract void onMessage(String var1);

    public abstract void onClose(int var1, String var2, boolean var3);

    public abstract void onError(Exception var1);

    public void onMessage(ByteBuffer bytes) {
    }

    public void setProxy(Proxy proxy) {
        if (proxy == null) {
            throw new IllegalArgumentException();
        } else {
            this.proxy = proxy;
        }
    }

    /** @deprecated */
    @Deprecated
    public void setSocket(Socket socket) {
        if (this.socket != null) {
            throw new IllegalStateException("socket has already been set");
        } else {
            this.socket = socket;
        }
    }

    public void setSocketFactory(SocketFactory socketFactory) {
        this.socketFactory = socketFactory;
    }

    public void sendFragmentedFrame(Opcode op, ByteBuffer buffer, boolean fin) {
        this.engine.sendFragmentedFrame(op, buffer, fin);
    }

    public boolean isOpen() {
        return this.engine.isOpen();
    }

    public boolean isFlushAndClose() {
        return this.engine.isFlushAndClose();
    }

    public boolean isClosed() {
        return this.engine.isClosed();
    }

    public boolean isClosing() {
        return this.engine.isClosing();
    }

    public boolean hasBufferedData() {
        return this.engine.hasBufferedData();
    }

    public void close(int code) {
        this.engine.close(code);
    }

    public void close(int code, String message) {
        this.engine.close(code, message);
    }

    public void closeConnection(int code, String message) {
        this.engine.closeConnection(code, message);
    }

    public void send(ByteBuffer bytes) {
        this.engine.send(bytes);
    }

    public void sendFrame(Framedata framedata) {
        this.engine.sendFrame(framedata);
    }

    public void sendFrame(Collection<Framedata> frames) {
        this.engine.sendFrame(frames);
    }

    public InetSocketAddress getLocalSocketAddress() {
        return this.engine.getLocalSocketAddress();
    }

    public InetSocketAddress getRemoteSocketAddress() {
        return this.engine.getRemoteSocketAddress();
    }

    public String getResourceDescriptor() {
        return this.uri.getPath();
    }

    public boolean hasSSLSupport() {
        return this.socket instanceof SSLSocket;
    }

    public SSLSession getSSLSession() {
        if (!this.hasSSLSupport()) {
            throw new IllegalArgumentException("This websocket uses ws instead of wss. No SSLSession available.");
        } else {
            return ((SSLSocket)this.socket).getSession();
        }
    }

    public IProtocol getProtocol() {
        return this.engine.getProtocol();
    }

    private void handleIOException(IOException e) {
        if (e instanceof SSLException) {
            this.onError(e);
        }

        this.engine.eot();
    }

    private class WebsocketWriteThread implements Runnable {
        private final WebSocketClient2 webSocketClient;

        WebsocketWriteThread(WebSocketClient2 webSocketClient) {
            this.webSocketClient = webSocketClient;
        }

        public void run() {
            Thread.currentThread().setName("WebSocketWriteThread-" + Thread.currentThread().getId());

            try {
                this.runWriteData();
            } catch (IOException var5) {
                IOException e = var5;
                WebSocketClient2.this.handleIOException(e);
            } finally {
                this.closeSocket();
            }

        }

        private void runWriteData() throws IOException {
            while(true) {
                try {
                    if (!Thread.interrupted()) {
                        ByteBuffer bufferx = (ByteBuffer)WebSocketClient2.this.engine.outQueue.take();
                        WebSocketClient2.this.ostream.write(bufferx.array(), 0, bufferx.limit());
                        //WebSocketClient2.this.ostream.flush();
                        continue;
                    }
                } catch (InterruptedException var4) {
                    Iterator var2 = WebSocketClient2.this.engine.outQueue.iterator();

                    while(var2.hasNext()) {
                        ByteBuffer buffer = (ByteBuffer)var2.next();
                        WebSocketClient2.this.ostream.write(buffer.array(), 0, buffer.limit());
                        //WebSocketClient2.this.ostream.flush();
                    }

                    Thread.currentThread().interrupt();
                }

                return;
            }
        }

        private void closeSocket() {
            try {
                if (WebSocketClient2.this.socket != null) {
                    WebSocketClient2.this.socket.close();
                }
            } catch (IOException var2) {
                IOException ex = var2;
                WebSocketClient2.this.onWebsocketError(this.webSocketClient, ex);
            }

        }
    }
}
