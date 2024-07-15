import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';
import 'package:learn/want_keep_alive.dart';

import 'bugfix_nest_scroll_view.dart' as bufgix;

/**
 *  nexted_scroll_view下面代码有bug，
 *  1.9999999999999999 2.0
 *  不相等导致位置未被恢复
 *  @override
    void restoreScrollOffset() {
    if (coordinator.canScrollBody) {
    super.restoreScrollOffset();
    }
    }
    https://book.flutterchina.club/chapter6/nestedscrollview.html#_6-12-1-nestedscrollview

    physics: const ClampingScrollPhysics(),
    要确认内部的可滚动组件（body）的 physics 是否需要设置为 ClampingScrollPhysics。比如上面的示例运行在 iOS 中时，ListView 如果没有设置为 ClampingScrollPhysics，则用户快速滑动到顶部时，会执行一个弹性效果，此时 ListView 就会与 header 显得割裂（滑动效果不统一），所以需要设置。但是，如果 header 中只有一个 SliverAppBar 则不应该加，因为 SliverAppBar 是固定在顶部的，ListView 滑动到顶部时上面已经没有要继续往下滑动的元素了，所以此时出现弹性效果是符合预期的。
    内部的可滚动组件（body的）不能设置 controller 和 primary，这是因为 NestedScrollView 的协调器中已经指定了它的 controller，如果重新设定则协调器将会失效。
    #
 * */

class DiscoverPage6 extends StatefulWidget {
  const DiscoverPage6({super.key});

  @override
  State<StatefulWidget> createState() {
    return DiscoverPageState6();
  }
}

class DiscoverPageState6 extends State<DiscoverPage6> {
  final controllerPage = PageController(keepPage: true);
  final ScrollController main = ScrollController();
  final List<ScrollController> scrollController = [
    ScrollController(),
    ScrollController(),
    ScrollController(),
    ScrollController()
  ];

  final bucket = PageStorageBucket();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: createBody(),
      floatingActionButton: ElevatedButton(
          onPressed: () {
            setState(() {});
            Future.delayed(Duration(milliseconds: 300), () {
              main.jumpTo(0);
              controllerPage.jumpToPage(0);
            });
          },
          child: Text("刷新页面")),
    );
  }

  Widget createBody() {
    debugPrint("hello build");
    return bufgix.NestedScrollView(
      forceRestoreScrollOffset: true,
      controller: main,
      headerSliverBuilder: (BuildContext context, bool innerBoxIsScrolled) {
        // 返回一个 Sliver 数组给外部可滚动组件。
        return <Widget>[
          bufgix.SliverOverlapAbsorber(
            handle:
                bufgix.NestedScrollView.sliverOverlapAbsorberHandleFor(context),
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
            key: const PageStorageKey<String>("pageView"),
            controller: controllerPage,
            onPageChanged: (int value) {},
            itemBuilder: (context, pageIndex) {
              debugPrint("inner build inner pag ${pageIndex}");
              return Column(
                children: [
                  Container(
                    height: 50,
                    color: Colors.blue,
                    child: Text("5 inner banner ${pageIndex}"),
                  ),
                  Expanded(
                      child: PageStorage(
                          bucket: bucket,
                          child: CustomScrollView(
                            controller:
                                PrimaryScrollController.maybeOf(context),
                            physics: const ClampingScrollPhysics(),
                            restorationId: "discover_restore_${pageIndex}",
                            key: PageStorageKey<String>(
                                "discover_customScroll_${pageIndex}"),
                            // scroll controll
                            // controller: scrollController[pageIndex],
                            slivers: <Widget>[
                              bufgix.SliverOverlapInjector(
                                handle: bufgix.NestedScrollView
                                    .sliverOverlapAbsorberHandleFor(context),
                              ),
                              SliverPadding(
                                padding: const EdgeInsets.all(8.0),
                                sliver: buildSliverList(pageIndex, 50),
                              ),
                            ],
                          )))
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
      itemExtent: 100,
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
