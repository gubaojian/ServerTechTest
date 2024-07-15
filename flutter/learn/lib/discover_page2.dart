import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:learn/want_keep_alive.dart';

class DiscoverPage2 extends StatefulWidget {
  const DiscoverPage2({super.key});

  @override
  State<StatefulWidget> createState() {
    return DiscoverPageState2();
  }
}

class DiscoverPageState2 extends State<DiscoverPage2> {
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
          SliverAppBar(
            title: const Text('嵌套ListView'),
            pinned: true, // 固定在顶部
            forceElevated: innerBoxIsScrolled,
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
      body: PageView.builder(
        controller: controllerPage,
        onPageChanged: (int value) {},
        itemBuilder: (context, pageIndex) {
          return CustomScrollView(
            key: PageStorageKey<String>("discover_customScroll_${pageIndex}"),
            // scroll controll
            // controller: scrollController[pageIndex],
            slivers: <Widget>[
              SliverOverlapInjector(
                handle:
                    NestedScrollView.sliverOverlapAbsorberHandleFor(context),
              ),
              SliverPadding(
                padding: const EdgeInsets.all(8.0),
                sliver: buildSliverList(pageIndex, 50),
              ),
            ],
          );
        },
        itemCount: 4,
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
