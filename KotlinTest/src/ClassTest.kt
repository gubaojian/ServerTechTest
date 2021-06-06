package hello

class Runoob{

    var name: String? = null
    var url: String? = null
    var city: String? = null


    fun foo(){
        print("Foo")
    }
}

class Person constructor(_firstName:String){


    constructor(name:String, alexa:Int):this(name){
        println("Alexa 拍名 $alexa");
    }


    var firstName = _firstName

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

    init{
        println("FirstName is $firstName")
    }


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

    var p2:Person = Person("World", 100)

}