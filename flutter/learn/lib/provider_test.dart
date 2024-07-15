import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class MyModel  extends ChangeNotifier {
    String title = "Hello world";

    void updateValue() {
      title = "Updated Success";
      notifyListeners();
    }
}

class ProviderTest extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
        create: (_) => MyModel(),
        builder: (context, child) {
          return Scaffold(
            body: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                ProviderElement(),
                FilledButton(onPressed: () {
                  Provider.of<MyModel>(context, listen: false).updateValue();
                  //context.read()<MyModel>().updateValue();
                }, child: Text("Provider"))
              ],
            ),
          );
        },
    );
  }
}

class ProviderElement extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    MyModel model = context.watch<MyModel>();
    return Text(model.title);
  }

}