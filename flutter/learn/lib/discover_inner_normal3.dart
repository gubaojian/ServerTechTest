import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:learn/want_keep_alive.dart';

class DiscoverPageInnerNormal3 extends StatefulWidget {
  const DiscoverPageInnerNormal3({super.key});

  @override
  State<StatefulWidget> createState() {
    return DiscoverPageInnerNormal3State();
  }
}

class DiscoverPageInnerNormal3State extends State<DiscoverPageInnerNormal3> {
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
              color: Colors.green,
              height: 200,
            ),
          ),
          SliverToBoxAdapter(
            child: Container(
              color: Colors.yellow,
              height: 200,
            ),
          ),
          //SliverPersistentHeader(delegate: delegate)
          //buildSliverList(5), //构建一个 sliverList
        ];
      },
      body: CustomScrollView(
        slivers: [
          SliverToBoxAdapter(
            child: Container(
              color: Colors.purple,
              height: 200,
            ),
          ),
          SliverGrid.builder(
              gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
                crossAxisCount: 2,
              ),
              itemBuilder: (BuildContext context, int index) {
                return SizedBox(
                  height: 100,
                  child: Center(
                      child:
                          Text('Item $index', style: TextStyle(fontSize: 24))),
                );
              })
        ],
      ),
    );
  }
}
