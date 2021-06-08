package inout

class Runoob<out A>(val a:A){
    fun foo(): A{
        return a
    }
}

class RunoobIn<in A>(a: A){
    fun foo(){
    }
}

class A<T>(val t:T, val t2 : T, val t3: T)
class Apple(var name:String)


fun main(args:Array<String>){
    var strCo: Runoob<String> = Runoob("A")
    var anyCo : Runoob<Any> = Runoob<Any>("b")
    println(anyCo.foo())

    anyCo = strCo

    println(anyCo.foo())

    println(strCo.foo())

    var strDCo = RunoobIn("a")
    var anyDCo = RunoobIn<Any>("b")


    println(strDCo.foo())

    strDCo = anyDCo

    println(strDCo.foo())

    var a1:A<*> = A(12, "String", Apple("苹果"))
    var a2:A<Any?> = A(12, "String", Apple("苹果"))

    val apple = a1.t3

    println(apple)

    val apple2:Apple = apple as Apple

    println(apple2.name)

    val l:ArrayList<*> = arrayListOf("String", 1, 1.2f, Apple("苹果"))

    for(item in l){
        println(item)
    }
}