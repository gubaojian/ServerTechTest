package user

class User(var name:String)

fun User.Print(){
    print("用户名 $name")
}

fun MutableList<Int>.swap(index1:Int, index2:Int){
    val tmp = this[index1]
    this[index1] = this[index2]
    this[index2] = tmp
}

open class C{
    fun foo(){
        println("成员函数")
    }
}

class D:C()

fun C.foo() = "c"

fun D.foo() = "d"

fun printFoo(c:C){
    println(c.foo())
}

fun Any?.toString(): String{
    if(this == null) return "null"
    return toString()
}


val <T> List<T>.lastIndex:Int
     get() = size - 1


class MyClass{
    companion object{}
}

fun MyClass.Companion.foo(){
    println("伴随对象的扩展函数")
}

val MyClass.Companion.no: Int
    get() = 10

fun main(args:Array<String>){
    var user = User("Runoob")
    user.Print()

    val l = mutableListOf(1, 2, 3)

    l.swap(1, 2)

    println(l.toString())

    printFoo(D())

    var t = null

    println(t.toString())

    println("no:${MyClass.no}")
    MyClass.foo()


}