package hello

import java.io.File;
import java.lang.IllegalArgumentException
import java.util.*

class Greeter(val name:String){
    fun greet(){
        println("Hello, $name");
    }
}

fun sum(a:Int, b:Int):Int{
    return a + b;
}

fun sumAuto(a:Int, b:Int) = a + b

fun printSum(a:Int, b:Int):Unit{
   println(a + b);
}

public fun printSumP(a:Int, b:Int){
    println(a + b);
}

fun printVars(vararg v:Int){
    for(vt in v){
        print(vt);
    }
}


fun parseInt(str:String?):Int?{
    return  str?.toInt();
}

fun getStringLength(obj:Any):Int?{
    if(obj is String){
        return  obj.length;
    }
    return null;
}

fun getStringLength2(obj:Any):Int?{
    if(obj !is String){
        return null
    }
    return obj.length;
}

fun decimalDigitValue(c:Char):Int{
    if(c !in '0'..'9'){
        throw   IllegalArgumentException("Out of range $c")
    }
    return c.toInt() - '0'.toInt();
}

fun main(args:Array<String>){
    println("Hello world");
    Greeter("Gubaojian").greet();

    var folders = File(".").listFiles()

    folders?.forEach { folder-> println(folder) }

    var a = 10;
    var b = 30;

    println(sum(a, b));

    println(sumAuto(a, b));

    printSum(a, b)

    printSumP(a, b)

    printVars(1, 2, 3, 4, 5, 6)

    val sumLambda :(Int, Int)->Int={x,y->x + y}

    println(sumLambda(1, 2));

    var a1:Int = 2;
    var b1 = 1;
    var c:Int;
    c = 1;

    var x = 5;
    x+=1;


    // 这是一个单行注释
    /**
     * 这是一个多行的块注释
     * */
    var d = 1;

    val s = "d is $d";
    d = 2;
    println(s);

    val s2 ="${s.replace("is","was")}, but now is $d";

    println(s2);

    var age:String? = null;
    val ages:String? = "23";

    var anum  = age?.toInt();
    var a2 = ages!!.toInt();

    println(anum)

    println(a2);


    println(parseInt(age));
    println(parseInt(ages));

    var a3 = parseInt(age);
    var b3 = parseInt(ages);
    if(a3 != null && b3 != null) {
        var r = a3 * b3;
        println(r);
    }else{
        println("result is null");
    }


    println(getStringLength("Love"))
    println(getStringLength(2))
    println(getStringLength2(2))

    println(getStringLength2("Love"))

    for(i in 1..4){
        println(i)
    }

    for(i in 4..1){
        println(i)
    }

    for(i in 1..10){
        println(i)
    }

    for(i in 1..4 step 2){
        println("step" + i)
    }

    for(i in 4 downTo  1 step 2){
        println("downTo " + i);
i   }

    for(i in 1 until 10){
        println("util $i");
    }

    println(Int.MAX_VALUE)
    println(Long.MAX_VALUE)

    val oneMillion = 1_000_000
    val creditCardNumber = 1234_5678_9012_3456L
    val socialSecurityNumber = 999_99_9999L
    val hexBytes = 0xFF_EC_DE_5E
    val bytes = 0b11010010_01101001_10010100_10010010

    println(bytes)

    val a5:Int = 10000
    println(a5 === a5)

    val boxedA:Int? = a5
    val anotherBoxedA:Int? = a5

    println(boxedA === anotherBoxedA)
    println(boxedA == anotherBoxedA)

    val b5:Byte = 1
    val i:Int = b5.toInt()


    println(" " + i + "  " + b5)

    val b6:Byte = 1
    val i6:Int = b5.toInt()


    println(" " + i6 + "  " + b6)

    val ch:Char = '1';


    println(ch.toInt())

    println(decimalDigitValue(ch))

    val aa = arrayOf(1, 2, 3)
    println(Arrays.toString(aa))

    val bb = Array(3, { i -> (i * 2) })

    println(Arrays.toString(bb))

    val str = "ddddddddd43255"
    for(c in str){
        println(c)
    }

    println(str[3])

    val text = """
        |ddddd
        |多行字符串
        |多行字符串
        |多行字符串
    """
    println(text)

    println(text.trimMargin())

    val i7= 10
    val s7 = "i = $i"

    println(s7)

    val s10 = "runoob"
    val str10 = "$s10.length is ${s10.length}"

    println(s10)

    println(str10)

    val price = """
        ${'$'}9.99
    """

   println(price)

}