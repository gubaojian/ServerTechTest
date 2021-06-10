package uitest

import kotlin.reflect.KProperty

var count:Int = 0

class MyDelegate<T>(t:T){
    operator fun getValue(thisRef:Any?, property:KProperty<*>):Int{

        return count++
    }
}

class ResourceLoader<T>(id:Int){
    operator fun provideDelegate(thisRef:MyUI, prop:KProperty<*>):MyDelegate<T?>{
        var t:T? = null
        return MyDelegate(t)
    }
}

fun <T> bindResource(id:Int):ResourceLoader<T>{
    return ResourceLoader<T>(id)
}

object ResourceID{
    val image_id:Int = 1
    val text_id:Int = 2
}

class MyUI{
    val image by bindResource<Int>(ResourceID.image_id)
    val text_id by bindResource<Int>(ResourceID.text_id)
}

fun main(args:Array<String>){
    val ui = MyUI()
    println("Hello world")
    println(ui.image)
    println(ui.text_id)
}