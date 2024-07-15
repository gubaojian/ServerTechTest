import 'dart:isolate';

import 'package:android_alarm_manager_plus/android_alarm_manager_plus.dart';
import 'package:flutter/material.dart';
import 'package:just_the_tooltip/just_the_tooltip.dart';
import 'package:learn/discover_page.dart';
import 'package:learn/one_page.dart';
import 'package:learn/provider_test.dart';

import 'discover_page2.dart';
import 'discover_page3.dart';
import 'discover_page4.dart';

// Be sure to annotate your callback function to avoid issues in release mode on Flutter >= 3.3.0
@pragma('vm:entry-point')
void printHello() {
  final DateTime now = DateTime.now();
  final int isolateId = Isolate.current.hashCode;
  print(
      "[$now] Timer Timer Hello, world! isolate=${isolateId} function='$printHello'");
}

Future<void> main() async {
  runApp(const MyApp());
  print("Init Timer");
  final int helloAlarmID = 0;
  await AndroidAlarmManager.periodic(
      const Duration(seconds: 10), helloAlarmID, printHello);
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // TRY THIS: Try running your application with "flutter run". You'll see
        // the application has a purple toolbar. Then, without quitting the app,
        // try changing the seedColor in the colorScheme below to Colors.green
        // and then invoke "hot reload" (save your changes or press the "hot
        // reload" button in a Flutter-supported IDE, or press "r" if you used
        // the command line to start the app).
        //
        // Notice that the counter didn't reset back to zero; the application
        // state is not lost during the reload. To reset the state, use hot
        // restart instead.
        //
        // This works for code too, not just values: Most code changes can be
        // tested with just a hot reload.
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
      routes: {
        "/one": (context) => OnePage(),
        "/test": (context) => ProviderTest(),
        "/discover": (context) => DiscoverPage(),
        "/discover2": (context) => DiscoverPage2(),
        "/discover3": (context) => DiscoverPage3(),
        "/discover4": (context) => DiscoverPage4(),
        "/home": (context) => MyHomePage(title: "测试"),
      },
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;

  final tooltipController = JustTheController();

  void _incrementCounter() {
    Navigator.of(context).pushNamed("/one");
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    return Scaffold(
      appBar: AppBar(
        // TRY THIS: Try changing the color here to a specific color (to
        // Colors.amber, perhaps?) and trigger a hot reload to see the AppBar
        // change color while the other colors stay the same.
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        // Here we take the value from the MyHomePage object that was created by
        // the App.build method, and use it to set our appbar title.
        title: Text(widget.title),
      ),
      body: Center(
        // Center is a layout widget. It takes a single child and positions it
        // in the middle of the parent.
        child: Column(
          // Column is also a layout widget. It takes a list of children and
          // arranges them vertically. By default, it sizes itself to fit its
          // children horizontally, and tries to be as tall as its parent.
          //
          // Column has various properties to control how it sizes itself and
          // how it positions its children. Here we use mainAxisAlignment to
          // center the children vertically; the main axis here is the vertical
          // axis because Columns are vertical (the cross axis would be
          // horizontal).
          //
          // TRY THIS: Invoke "debug painting" (choose the "Toggle Debug Paint"
          // action in the IDE, or press "p" in the console), to see the
          // wireframe for each widget.
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Text(
              'You have pushed the button this many times:',
            ),
            Text(
              '$_counter',
              style: Theme.of(context).textTheme.headlineMedium,
            ),
            JustTheTooltip(
              controller: tooltipController,
              isModal: true,
              preferredDirection: AxisDirection.up,
              child: Material(
                color: Colors.grey.shade800,
                shape: const CircleBorder(),
                elevation: 4.0,
                child: Padding(
                  padding: EdgeInsets.all(8.0),
                  child: GestureDetector(
                    onTap: () {
                      tooltipController.showTooltip();
                    },
                    child: Padding(
                      padding: EdgeInsets.only(right: 100),
                      child: Icon(
                        Icons.add,
                        color: Colors.red,
                      ),
                    ),
                  ),
                ),
              ),
              content: const Padding(
                padding: EdgeInsets.all(8.0),
                child: Text(
                  'Bacon ipsum dolor amet kevin turducken brisket pastrami, salami ribeye spare ribs tri-tip sirloin shoulder venison shank burgdoggen chicken pork belly. Short loin filet mignon shoulder rump beef ribs meatball kevin.',
                ),
              ),
            ),
            ElevatedButton(
                onPressed: () {
                  Navigator.of(context).pushNamed("/test");
                },
                child: Text("Provider")),
            ElevatedButton(
                onPressed: () {
                  Navigator.of(context).pushNamed("/discover");
                },
                child: Text("Discover")),
            ElevatedButton(
                onPressed: () {
                  Navigator.of(context).pushNamed("/discover2");
                },
                child: Text("Discover2 PageView and float appbar")),
            ElevatedButton(
                onPressed: () {
                  Navigator.of(context).pushNamed("/discover3");
                },
                child: Text("Discover3 Single Float Header")),
            ElevatedButton(
                onPressed: () {
                  Navigator.of(context).pushNamed("/discover4");
                },
                child: Text("Discover3 Single & Inner Float Header")),
            TextField(
              keyboardType: TextInputType.multiline,
              minLines: 1, //Normal textInputField will be displayed
              maxLines: 5, // when user presses enter it will adapt to it
            ),
            ElevatedButton(
                onPressed: () {
                  _layoutTest();
                },
                child: Text("LayoutText")),
            SizedBox(
              width: 279,
              child: Text(
                'Grab azgo\'s 1-for-1 limited Deals. Subscribe now before you miss them!',
                style: const TextStyle(
                    fontSize: 14,
                    fontWeight: FontWeight.w400,
                    color: Colors.red,
                    height: 17.5 / 14),
              ),
            )
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _incrementCounter,
        tooltip: 'Increment',
        child: const Icon(Icons.add),
      ), // This trailing comma makes auto-formatting nicer for build methods.
    );
  }

  void _layoutTest() {
    final style = const TextStyle(
        fontSize: 14,
        fontWeight: FontWeight.w400,
        color: Colors.white,
        height: 17.5 / 14);
    final painter = TextPainter(
        textDirection: TextDirection.ltr,
        text: TextSpan(
            text:
                'Grab azgo\'s 1-for-1 limited Deals. Subscribe now before you miss them!',
            style: style,
            children: []));
    painter.layout(maxWidth: 279);
    print("painter ${painter.height} ");
    final numLines = painter.computeLineMetrics().length;
    print("painer ${numLines} ${painter.computeLineMetrics()} ${painter.size}");
  }
}
