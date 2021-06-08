package box

class Box<T>(t:T){
    var value = t
}

fun <T> boxIn(value:T) = Box(value)

val box4 = boxIn(1)
val box5 = boxIn<Int>(2)

fun <T> doPrint(content:T){
    when(content){
        is Int -> println("整型数字为 ${content}")
        is String -> println("字符串转换为大写: ${content.toUpperCase()}")
        else -> println("T 不是整型，也不是字符串")
    }
}

fun <T: Comparable<T>> sort2(list: List<T>){

}

fun <T> copyWhenGreater(list: List<T>, threshold:T): List<String>
    where T : CharSequence,
         T : Comparable<T>{
         return list.filter {it > threshold}.map{it.toString()}
}

fun main(args:Array<String>){
    var boxInt = Box<Int>(10)
    var boxString = Box<String>("Runoob")

    println(boxInt.value)

    println(boxString.value)

    val age = 23
    val name = "runoob"
    val bool = true

    doPrint(age)
    doPrint(name)
    doPrint(bool)

}