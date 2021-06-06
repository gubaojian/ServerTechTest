package hello

open class Base{
    open fun f(){}
}

abstract class Derived : Base() {
    override  abstract fun f()
}

class Outer{
    private val bar: Int = 1

    var v = "Outer成员属性"

    class Nested{
        fun foo() = 2
    }

    inner class Inner{
        fun foo() = bar
        fun innerTest(){
            var o = this@Outer
            println("匿名类可以引用外部类的成员 " + o.v);
        }
    }
}

fun main(args:Array<String>){
    val demo = Outer.Nested().foo()
    println(demo)

    val id = Outer().Inner().foo()
    println(id)
    val id2 = Outer().Inner().innerTest()
    println(id2)
}