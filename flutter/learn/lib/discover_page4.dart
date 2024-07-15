import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:learn/want_keep_alive.dart';

class DiscoverPage4 extends StatefulWidget {
  const DiscoverPage4({super.key});

  @override
  State<StatefulWidget> createState() {
    return DiscoverPageState4();
  }
}

class DiscoverPageState4 extends State<DiscoverPage4> {
  final controllerPage = PageController(keepPage: true);
  final ScrollController main = ScrollController();
  final List<ScrollController> scrollController = [
    ScrollController(),
    ScrollController(),
    ScrollController(),
    ScrollController()
  ];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: createBody(),
    );
  }

  Widget createBody() {
    debugPrint("hello build");
    return NestedScrollView(
      headerSliverBuilder: (BuildContext context, bool innerBoxIsScrolled) {
        // 返回一个 Sliver 数组给外部可滚动组件。
        return <Widget>[
          SliverOverlapAbsorber(
            handle: NestedScrollView.sliverOverlapAbsorberHandleFor(context),
            sliver: SliverToBoxAdapter(
              child: Container(
                color: Colors.green,
                height: 200,
              ),
            ),
          ),
          /**
              SliverToBoxAdapter(
              child: Container(
              color: Colors.red,
              height: 200,
              ),
              )*/
          //SliverPersistentHeader(delegate: delegate)
          //buildSliverList(5), //构建一个 sliverList
        ];
      },
      body: Column(
        children: [
          Container(
            color: Colors.yellow,
            height: 100,
          ),
          Expanded(
              child: PageView.builder(
            controller: controllerPage,
            onPageChanged: (int value) {},
            itemBuilder: (context, pageIndex) {
              debugPrint("hello build inner pag ${pageIndex}");
              return Column(
                children: [
                  Container(
                    height: 50,
                    color: Colors.blue,
                    child: Text("inner banner ${pageIndex}"),
                  ),
                  Expanded(
                      child: CustomScrollView(
                    key: PageStorageKey<String>(
                        "discover_customScroll_${pageIndex}"),
                    // scroll controll
                    // controller: scrollController[pageIndex],
                    slivers: <Widget>[
                      SliverOverlapInjector(
                        handle: NestedScrollView.sliverOverlapAbsorberHandleFor(
                            context),
                      ),
                      SliverPadding(
                        padding: const EdgeInsets.all(8.0),
                        sliver: buildSliverList(pageIndex, 50),
                      ),
                    ],
                  ))
                ],
              );
            },
            itemCount: 4,
          ))
        ],
      ),
    );
  }

  // 构建固定高度的SliverList，count为列表项属相
  Widget buildSliverList(int pageIndex, int count) {
    return SliverFixedExtentList(
      itemExtent: 50,
      delegate: SliverChildBuilderDelegate(
        (context, index) {
          return Container(
            height: 100,
            child: ListTile(title: Text('${pageIndex} at $index')),
          );
        },
        childCount: count,
      ),
    );
  }
}
