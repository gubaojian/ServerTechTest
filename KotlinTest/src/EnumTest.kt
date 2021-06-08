package enumtest

enum class Color(val rgb:Int){
    RED(0xff0000),
    BLACK(0x00FF00),
    BLUE(0x0000FF)
}

enum class Shape(value:Int){
    ovel(100),
    rectangle(200)
}

enum class ProtocolState{
    WAITING{
        override fun signal() = TALKING
    },
    TALKING{
      override fun signal() = WAITING
    };

    abstract fun signal(): ProtocolState
}

inline fun <reified T : Enum<T>> printAllValues(){
    print(enumValues<T>().joinToString { it.name })
}

fun main(args:Array<String>){
    var color:Color = Color.BLUE

    println(Color.values())

    println(Color.valueOf("RED"))

    println(color.name)
    println(color.ordinal)
}
