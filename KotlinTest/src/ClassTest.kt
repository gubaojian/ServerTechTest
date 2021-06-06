package hello

class Runoob{

    var name: String? = null
    var url: String? = null
    var city: String? = null


    fun foo(){
        print("Foo")
    }
}

class Person constructor(firstName:String){

    var lastName:String = "zhang"
        get() = field.toUpperCase()
        set
    var no:Int = 100
        get() = field
        set(value){
            if(value < 10){
                field = value
            }else{
                field = -1
            }
        }
    var height:Float = 145.4f
        private set

}

class Empty

fun main(args:Array<String>){
    println("hello world");

    val site = Runoob();
    println("site name ${site.name}");

    var person:Person = Person("First Name")
    person.lastName = "wang"

    println("lastName:${person.lastName}");

    person.no = 9
    println("no:${person.no}")

    person.no = 20
    println("no: ${person.no}")

}