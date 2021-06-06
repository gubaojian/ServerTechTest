package hello

import java.util.*


fun hasPrefix(x:Any)= when(x){
    is String -> x.startsWith("prefix")
    else -> false
}

fun main(args:Array<String>){
    println(Arrays.toString(args))

    var a = 10
    var b = 20
    var max = a
    if(a < b) max = b

    var max2:Int
    if(a > b){
        max2 = a
    }else{
        max2 = b
    }

    println("$max2 max2 max $max" )

    val max3 = if(a > b) a else b

    println("max3 $max3")

    val max4 = if(a > b){
        println("Choose a")
        a
    }else{
        println("Choose b")
        b
    }

    var x = 0
    if(x > 0){
        println("x 大于 0")
    }else if(x == 0){
        println("x 等于 0")
    }else{
        println("x 小于 0")
    }

    var a1 = 1
    var b1 = 2
    val c1 = if(a1 >= b1 ) a1 else b1

    println("c1 的值为 $c1");

    val x1 = 5
    val y1 = 9

    if(x1 in 1..8){
        println("x1 在区间内")
    }

    when(x){
        1 -> println("x == 1")
        2 -> println("x == 2")
        else -> {
            print("x 不是1，也不是2")
        }
    }

    when(x){
        0,1 -> println("x == 0 or x == 1")
        else ->print("otherwise")
    }

    when(x){
        in 1..10 -> print("x is in the range")
        in 20..30 -> print("x is valid")
        !in 10..30 ->println("x is outside the range$x")
        else -> print("none of the above")
    }


    when(x){
        in 0..10 -> println("x 在该范围内")
        else -> println("x 不在该范围内")
    }

    val items = setOf("apple", "banana", "orangle5")

    when{
        "orangle" in items -> println("orangle")
        "apple" in items -> println("apple is fine too")
    }


    for(item in items){
        println("item is $item")
    }

    for(item:String in items){
        println("item string value is $item")
    }

    for(i in items.indices){
        println("index $i value $items[i]")
    }

    for((index, value) in items.withIndex()){
        println("the element at $index is $value")
    }

    var arrays = listOf("apple", "banana", "kiwi")
    for(item in arrays){
        println(item + " is item")
    }

    for(index in arrays.indices){
        println("item ar $index is ${arrays[index]}")
    }

    println("-----while使用-----");
    var x5 = 5;
    while(x5 > 0){
        println(x5--)
    }

    println("-----do...while使用-----")
    var y = 5
    do{
        println(y--)
    }while(y > 0)

    for(i in 1..10){
        if(i == 3) continue
        println("i ===  $i")
        if(i > 5) break
    }

    loop@ for(i in 1..100){
        for(j in 1..100){
            if(j == 50){
                println("$i $j")
                break@loop
            }
        }
    }
    println("dddd");

    var ints = listOf<Int>(2, 4, 3, 2, 3, 0, 1)

    fun foo(){
        ints.forEach({
            if(it == 0) return
            println(it)
        })
    }
    foo()

    println("ccccccccc")

    fun foo2(){
        ints.forEach(fun(value:Int){
            if(value == 0) return
            println(value)
        })
    }
    foo2()

    ints.forEach{
        if(it == 0) return@forEach
        println("item  $it")
    }

    ints.forEach lit@{
        if(it == 0) return@lit
        println(it)
    }

    /**
    ints.forEach{
        if(it == 0) return
        println(it)
    }*/

    println("main end")
  

}
