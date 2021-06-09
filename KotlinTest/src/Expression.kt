package exptest

import java.awt.event.MouseAdapter
import java.awt.event.MouseEvent
import java.util.*
import javax.swing.JComponent
import kotlin.collections.ArrayList


var mse = object:MouseAdapter(){

    override fun mouseClicked(e: MouseEvent?) {
        super.mouseClicked(e)
    }

    override fun mouseEntered(e: MouseEvent?) {
        super.mouseEntered(e)
    }
};

open class A(x:Int){
    public open val y: Int = x
}

interface  B{
}

val ab : A = object :A(1),B{
    override val y = 15
}

class C{
    private fun foo() = object{
        val x: String = "x"
    }

    fun publicFoo() = object{
        val x: String = "x"
    }

    fun bar(){
        val x1 = foo().x
       // val x2 = publicFoo().x
    }
}

fun countClicks(window:JComponent){
    var clickCount = 0
    var enterCount = 0

    window.addMouseListener(object: MouseAdapter(){
        override fun mouseClicked(e: MouseEvent?) {
            super.mouseClicked(e)
            clickCount++
        }

        override fun mouseEntered(e: MouseEvent?) {
            super.mouseEntered(e)
            enterCount++
        }
    })
}

class DataProvider

object DataProviderManager{
    var name: String? = null

    fun registerDataProvider(provider:DataProvider){

    }

    val allDataProviders : MutableList<DataProvider> = ArrayList()

}

object SiteLove{
    var url:String = ""
    val name: String = "菜鸟教程"

    object Desktop{
        var url = "www.runoob.com"

        fun showName(){
            println("desk legs $name")
        }
    }
}

object DefaultListener : MouseAdapter(){
    override fun mouseEntered(e: MouseEvent?) {
        super.mouseEntered(e)
    }

    override fun mouseClicked(e: MouseEvent?) {
        super.mouseClicked(e)
    }
}

interface Factory<T>{
    fun create():T
}

class MyClass{
    companion object : Factory<MyClass>{
        override fun create(): MyClass = MyClass()
    }
}

fun main(args:Array<String>){
    val site = object {
        var name: String = "菜鸟教程"
        var url: String = "www.runoob.com"
    }

    println(site.name)
    println(site.url)

    var dataProvider = DataProvider()

    DataProviderManager.registerDataProvider(dataProvider)
    DataProviderManager.allDataProviders.add(dataProvider)

    println(DataProviderManager.allDataProviders.size)

    var data1 = DataProviderManager
    var data2 = DataProviderManager
    data1.name = "Test"

    println("data1 name ${data1.name} +  ${data2.name}")

    var s1 = SiteLove
    var s2 = SiteLove

    s1.url = "www.runoob.com"

    println(s1.url)
    println(s2.url)

    SiteLove.Desktop.url
    SiteLove.Desktop.showName()

    var instance = MyClass.create()

    //println(instance)

    MyClass.create()




}