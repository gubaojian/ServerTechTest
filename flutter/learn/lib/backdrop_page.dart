import 'dart:ui';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:learn/res/assets_res.dart';
import 'package:learn/want_keep_alive.dart';

class BackgropPage extends StatefulWidget {
  const BackgropPage({super.key});

  @override
  State<StatefulWidget> createState() {
    return BackgropPageState();
  }
}

class BackgropPageState extends State<BackgropPage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.green,
      appBar: AppBar(title: Text("GeeksforGeeks"), centerTitle: true),
      body: SingleChildScrollView(
        child: Column(
          children: [
            const SizedBox(height: 50),
            SizedBox(
              height: 300,
              width: double.infinity,
              child: Stack(
                children: [
                  Image.asset(AssetsRes.TEST, width: 200, height: 200),
                ],
              ),
            ),
            SizedBox(
              height: 300,
              width: 300,
              child: Stack(
                children: [
                  Image.asset(AssetsRes.TEST, width: 200, height: 200),
                  Positioned(
                      left: 40,
                      top: 10,
                      child: ClipRRect(
                        borderRadius: BorderRadius.only(
                          bottomLeft: Radius.circular(16),
                          topRight: Radius.circular(16),
                        ),
                        child: BackdropFilter(
                          filter: ImageFilter.blur(sigmaX: 10, sigmaY: 10),
                          child: Container(
                            height: 23,
                            padding: EdgeInsets.only(left: 8, right: 8),
                            decoration: BoxDecoration(
                                color: Colors.black.withOpacity(0.4),
                                borderRadius: BorderRadius.only(
                                  bottomLeft: Radius.circular(16),
                                  topRight: Radius.circular(16),
                                )),
                            alignment: Alignment.centerLeft,
                            child: Text("语文素养语文素养",
                                style: TextStyle(
                                    color: Colors.white,
                                    fontSize: 11,
                                    fontWeight: FontWeight.bold)),
                          ),
                        ),
                      ))
                ],
              ),
            ),
            SizedBox(
              height: 300,
              width: 300,
              child: Stack(
                children: [
                  Image.asset(AssetsRes.TEST, width: 200, height: 200),
                  Positioned(
                      left: 40,
                      top: 10,
                      child: Container(
                        height: 23,
                        padding: EdgeInsets.only(left: 8, right: 8),
                        decoration: BoxDecoration(
                            color: Colors.black.withOpacity(0.4),
                            borderRadius: BorderRadius.only(
                              bottomLeft: Radius.circular(16),
                              topRight: Radius.circular(16),
                            )),
                        alignment: Alignment.centerLeft,
                        child: Text("语文素养语文素养",
                            style: TextStyle(
                                color: Colors.white,
                                fontSize: 11,
                                fontWeight: FontWeight.bold)),
                      ))
                ],
              ),
            ),
            SizedBox(
              height: 300,
              width: 300,
              child: Stack(
                children: [
                  Image.asset(AssetsRes.TEST2, width: 200, height: 200),
                  Positioned(
                      left: 40,
                      top: 10,
                      child: ClipRRect(
                        borderRadius: BorderRadius.only(
                          bottomLeft: Radius.circular(16),
                          topRight: Radius.circular(16),
                        ),
                        child: BackdropFilter(
                          filter: ImageFilter.blur(sigmaX: 10, sigmaY: 10),
                          child: Container(
                            height: 23,
                            padding: EdgeInsets.only(left: 8, right: 8),
                            decoration: BoxDecoration(
                                color: Colors.black.withOpacity(0.4),
                                borderRadius: BorderRadius.only(
                                  bottomLeft: Radius.circular(16),
                                  topRight: Radius.circular(16),
                                )),
                            alignment: Alignment.centerLeft,
                            child: Text("语文素养语文素养",
                                style: TextStyle(
                                    color: Colors.white,
                                    fontSize: 11,
                                    fontWeight: FontWeight.bold)),
                          ),
                        ),
                      ))
                ],
              ),
            ),
            SizedBox(
              height: 300,
              width: 300,
              child: Stack(
                children: [
                  Image.asset(AssetsRes.TEST2, width: 200, height: 200),
                  Positioned(
                      left: 40,
                      top: 10,
                      child: ClipRRect(
                        borderRadius: BorderRadius.only(
                          bottomLeft: Radius.circular(16),
                          topRight: Radius.circular(16),
                        ),
                        child: Container(
                          height: 23,
                          padding: EdgeInsets.only(left: 8, right: 8),
                          decoration: BoxDecoration(
                              color: Colors.black.withOpacity(0.4),
                              borderRadius: BorderRadius.only(
                                bottomLeft: Radius.circular(16),
                                topRight: Radius.circular(16),
                              )),
                          alignment: Alignment.centerLeft,
                          child: Text("语文素养语文素养",
                              style: TextStyle(
                                  color: Colors.white,
                                  fontSize: 11,
                                  fontWeight: FontWeight.bold)),
                        ),
                      ))
                ],
              ),
            )
          ],
        ),
      ),
    );
  }
}
