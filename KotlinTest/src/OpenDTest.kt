package opend

open class D {

}

class D1 : D(){

}

open class C{
    open fun D.foo(){
        println("D.foo in C")
    }

    open fun D1.foo(){
        println("D1.foo in C")
    }

    fun caller(d:D){
        d.foo()
    }
}

class   C1 : C(){

    override  fun D.foo(){
        println("D.foo in C1")
    }

    override fun D1.foo(){
        println("D1.foo in C1")
    }

}

fun main(args:Array<String>){
    C().caller(D())
    C1().caller(D())
    C().caller(D1())
}