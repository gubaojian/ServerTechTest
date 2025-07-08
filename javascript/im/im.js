import { WKIM, WKIMChannelType, WKIMEvent } from 'easyjssdk';

// 1. 初始化
const im = WKIM.init("ws://127.0.0.1:5200", {
    uid: "your_user_id", // 当前连接的用户uid
    token: "your_auth_token" // 当前连接用户的认证token（默认不需要认证，如果开启了需要填写）
});

// 2. 监听
im.on(WKIMEvent.Connect, async () => {
    console.log("IM Connected!");
    //  发送消息
    const result = await im.send("target user",WKIMChannelType.Person,{ type: "text", content: "HelloEasyJSSDK!" })
});

// 监听接收消息事件
im.on(WKIMEvent.Message, (message) => {
    console.log("Received:", message);
});

// 监听错误事件
im.on(WKIMEvent.Error, (error) => {
    console.error("IM Error:", error);
});

// 3. 连接
await im.connect()
