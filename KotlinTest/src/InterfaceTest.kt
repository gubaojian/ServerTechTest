

interface MyInterface{
    var name:String
    fun bar()
    fun foo(){
        println("foo")
    }
}


class Child : MyInterface{

    override var name:String = "Impl"

    override fun bar(){
        //方法体
        println("bar")
    }
}

interface A {
    fun foo(){ println("A") }
    fun bar()
}

interface B {
    fun foo() { println("B") }
    fun bar() { println("bar")}
}

class C : A{
    override  fun bar(){
        println("bar")
    }
}

class D : A,B{
    override fun foo(){
        super<A>.foo()
        super<B>.foo()
    }

    override fun bar() {
        super<B>.bar()
    }
}



fun main(args:Array<String>){
    val c = Child()
    c.foo()
    c.bar()

    println(c.name)

    val d = D()
    d.foo()
    d.bar()
}