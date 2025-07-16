import ws from 'k6/ws';
var stop = false;
export default function() {
  ws.connect('ws://localhost:8080/ws', {}, (socket) => {
    socket.on('open', () => {
      for(var i=0; i<1024*10; i++) {
        socket.send('在这个信息爆炸的时代，我们每天都在接触各种各样的文字。中文的博大精深，英文的简洁明了，都有着各自独特的魅力。当两种语言交织在一起，往往能碰撞出不一样的火花。无论是阅读一篇优美的散文，还是浏览一则简短的新闻，文字都在向我们传递着信息和情感。Learning a new language is like opening a new door to the world. It allows us to communicate with people from different cultures and understand their perspectives. Chinese characters, with their rich history and meanings, are a treasure of human civilization. Each character tells a story, reflecting the wisdom and experiences of our ancestors. 在全球化的今天，中英文的切换使用已经成为一种常态。无论是在工作中还是生活中，我们都可能需要在两种语言之间自如转换。这不仅是一种技能，更是一种能力，让我们能够更好地适应这个多元化的世界。English, as an international language, connects people from all over the globe. It is the language of science, technology, business, and entertainment. By mastering English, we can access a vast amount of knowledge and opportunities that would otherwise be unavailable. 中文的表达细腻而丰富，能够精准地传达复杂的情感和思想。从古代的诗词歌赋到现代的网络文学，中文一直在不断发展和创新，展现出强大的生命力。每一个汉字都有着独特的结构和含义，组合在一起又能形成千变万化的表达。The beauty of language lies in its ability to evolve and adapt. Both Chinese and English have borrowed words and expressions from each other, enriching their vocabularies and making them more versatile. This cross-pollination is a testament to the interconnectedness of human societies. 无论是用中文还是英文，表达的核心都是传递信息、交流思想。语言是工具，也是桥梁，');
      }]
    });
    socket.setTimeout(function () {
          console.log('2 seconds passed, closing the socket')
          socket.close()
        }, 10000);
  });
}

export function teardown() {
   stop = true;
}