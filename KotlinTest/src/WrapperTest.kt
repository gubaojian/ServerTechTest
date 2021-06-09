package wrapper

import kotlin.reflect.KProperty
import kotlin.properties.Delegates



interface Base{
    fun print()

    fun print2()
}

class BaseImpl(val x:Int) : Base{
    override fun print(){
        println(x)
    }

    override fun print2() {
        println("hello world")
    }
}


class Derived(b: Base) : Base by b

class Example{
    var p: String by Delegate()
}

class Delegate{
    var value_:String?= null
    operator fun getValue(thisRef: Any?, property:KProperty<*>):String{
        return "$thisRef, 这里委托了 ${property.name} 属性 ${value_}"
    }

    operator fun setValue(thisRef: Any?, property: KProperty<*>, value:String){
        println("$thisRef 的 ${property.name} 值为 $value")
        value_ = value
    }

}

val lazyValue: String by lazy{
    println("computed")
    "Hello"
}

class User{
    var name: String by Delegates.observable("初始值"){
        prop, old, new ->
        println("旧值：$old -> 新值: $new")
    }
}

class Site(val map:MutableMap<String, Any?>){
    val name: String by map
    val url: String by map
}

class Foo{


    var notNullBar: String by Delegates.notNull<String>()

    fun isValid(): Boolean {
        return true
    }

    fun doSomeThing() {
        println("do some thing")
    }

}

fun example(computeFoo: () -> Foo){
    val memoizedFoo by lazy(computeFoo)

    if(false && memoizedFoo.isValid()){
        memoizedFoo.doSomeThing()
    }
}


fun main(args:Array<String>){
    val b = BaseImpl(10)
    val w = Derived(b);
    w.print()
    w.print2()

    val e = Example()

    println(e.p)

    e.p = "Runoob"

    println(e.p)

    println(lazyValue)

    println(lazyValue)

    var user = User()

    user.name = "第一次赋值"
    user.name = "第二次赋值"

    var map:MutableMap<String,Any?> = mutableMapOf(
        "name" to "菜鸟教程",
        "url"  to "www.runoob.com"
    )
    var site3 = Site(map)

    println(site3.name)
    println(site3.url)

    map.put("name", "Google")
    map.put("url", "www.google.com")

    println(site3.name)
    println(site3.url)

    var foo:Foo = Foo()
    foo.notNullBar = "bar"

    println(foo.notNullBar)

    example(fun():Foo{
        println("init foo")
        return Foo()
    })

}