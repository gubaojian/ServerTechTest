package hello4

class D {
    fun bar(){
        println("D bar")
    }
}

class C{
    fun bar(){
        println("C bar")
    }

    fun D.foo(){
        bar()
        this@C.bar()
    }

    fun caller(d:D){
        d.foo()
    }
}

fun main(args:Array<String>){
    val c: C = C()
    val d: D = D()
    c.caller(d)
}