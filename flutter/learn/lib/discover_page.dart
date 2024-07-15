import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:learn/want_keep_alive.dart';

class DiscoverPage extends StatefulWidget {
  const DiscoverPage({super.key});

  @override
  State<StatefulWidget> createState() {
    return DiscoverPageState();
  }
}

class DiscoverPageState extends State<DiscoverPage> {
  final controllerPage = PageController(keepPage: true);
  final List<ScrollController> scrollController = [
    ScrollController(),
    ScrollController(),
    ScrollController(),
    ScrollController()
  ];

  @override
  Widget build(BuildContext context) {
    return NestedScrollView(
      headerSliverBuilder: (BuildContext context, bool innerBoxIsScrolled) {
        // 返回一个 Sliver 数组给外部可滚动组件。
        return <Widget>[
          SliverToBoxAdapter(
            child: Container(
              color: Colors.red,
              height: 200,
            ),
          ),
          SliverAppBar(
            title: const Text('嵌套ListView'),
            pinned: true, // 固定在顶部
            forceElevated: innerBoxIsScrolled,
          ),
          SliverToBoxAdapter(
            child: Container(
              color: Colors.red,
              height: 200,
            ),
          ),
          //SliverPersistentHeader(delegate: delegate)
          //buildSliverList(5), //构建一个 sliverList
        ];
      },
      body: PageView.builder(
        controller: controllerPage,
        itemBuilder: (context, pageIndex) {
          return WantKeepAlive(
              key: PageStorageKey<String>("keep${pageIndex}"),
              ListView.builder(
                key: PageStorageKey<String>("list${pageIndex}"),
                //controller: scrollController[pageIndex],
                padding: const EdgeInsets.all(8),
                physics: const ClampingScrollPhysics(),
                //重要
                itemCount: 50,
                itemBuilder: (BuildContext context, int index) {
                  return SizedBox(
                    height: 100,
                    child: Center(
                        child: Text('page ${pageIndex} Item $index',
                            style: TextStyle(fontSize: 24))),
                  );
                },
              ));
        },
        itemCount: 4,
      ),
    );
  }
}
