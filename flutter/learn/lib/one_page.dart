import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:learn/controller/one_page_controller.dart';

class OnePage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return GetBuilder(
        init: OnePageController(),
        assignId : true,
        builder: (OnePageController controller){
          return Scaffold(
            body: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text('hello world ${controller.serverText}'),
                TextButton(
                  onPressed: () {
                    Navigator.of(context).pop();
                  },
                  child: Text('Pop'),
                ),
                ElevatedButton(
                  onPressed: () {
                    Navigator.of(context).pushNamed("/one");
                  },
                  child: Text('Push'),
                )
              ],
            ),
          );
        });
  }
  
}