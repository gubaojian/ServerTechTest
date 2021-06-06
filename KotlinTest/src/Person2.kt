package person2

open class  Person{

    open fun study(){
        println("我毕业啦")
    }
}

open class A{
    open fun f () { print("A") }
    fun a(){ print("a")}
}

interface B{
    fun f(){ print("B") }
    fun b() { print("b") }
}

class C():A(),B{
    override fun f(){
        super<A>.f();
        super<B>.f();
    }
}

class Student : Person(){
    override fun study(){
        println("我在读大学")
    }
}

open class Foo{
    open val x: Int
        get() {
            return 2
        }
}

class Bar2 : Foo(){
    override val x: Int
    get(){
        return 4
    }
}

interface Foo2{
    val count: Int
}

class Bar1(override  val count:Int):Foo2

class Bar3 : Foo2 {
    override var count: Int = 3
}


fun main(args:Array<String>){
    val s = Student();
    s.study()

    val c = C();
    c.f()
}