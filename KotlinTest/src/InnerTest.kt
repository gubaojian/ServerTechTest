package hello

class Test{
    var v = "成员属性"

    fun setInterFace(test:TestInterFace){
        test.test()
    }
}

annotation class Worker{

}

open class Like{

}

enum class Love{

}

final class Pat{

}

abstract class WP{

}

private fun foo(){

}

public var bar: Int = 5
internal val baz = 6

interface TestInterFace{
    fun test()
}

fun main(args:Array<String>){
    var test = Test()

    /**
     * 采用对象表达式创建接口对象，既匿名内部类的实例。
     * */
    test.setInterFace(object:TestInterFace{
      override fun test(){
          println("d对象表达式创建匿名内部类的实例");
      }
    })
}