import 'package:get/get.dart';

class OnePageController extends GetxController {

  String serverText = "request";

  @override
  void onInit() {
    super.onInit();
    print("hello world on init");
    Future.delayed(Duration(seconds: 2), (){
      serverText = "return after 2 seconds";
      update();
    });
  }

  @override
  void onClose() {
    super.onClose();
    print("hello world on closed");
  }


}