package hellobase

class Example;

open class Base(p:Int)

class Derived(p:Int):Base(p){

}

open class Person(var name:String, var age:Int){

}

class Student:Person{

    constructor(name:String, age:Int, no:String,  score:Int):super(name,age){

        println("$name");
        println("$age");
        println("$no");
        println("$score");

    }


}

open class Person2(name:String){
    constructor(name:String, age:Int):this(name){
        //初始化
        println("-----基础类初始化--------")
    }
}

fun main(args:Array<String>){
    var s:Student = Student("Runoob", 18, "S123456", 89)


    var p = Person2("Person", 10)

    println()
}