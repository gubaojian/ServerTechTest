package datatest

data class User(val name:String, val age:Int)

fun main(args:Array<String>){
   val jack = User(name="Jack", age = 1)
   val olderJack = jack.copy(age = 2)
   println(jack)
   println(olderJack)

    var jane = User("Jane", 35)
    val (name,age) = jane
    println("$name, $age years of age")
}