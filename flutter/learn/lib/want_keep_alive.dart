import 'package:flutter/material.dart';

class WantKeepAlive extends StatefulWidget {
  final Widget widget;

  const WantKeepAlive(this.widget, {super.key});

  @override
  State<WantKeepAlive> createState() => _WantKeepAliveState();
}

class _WantKeepAliveState extends State<WantKeepAlive>
    with AutomaticKeepAliveClientMixin {
  @override
  Widget build(BuildContext context) {
    super.build(context);
    return widget.widget;
  }

  @override
  bool get wantKeepAlive => true;
}
