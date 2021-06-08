sealed class UiOp{
    object Show:UiOp()
    object Hide:UiOp()
    class TransitionX(val px:Float): UiOp()
    class TransitionY(val px:Float): UiOp()
}

class View{

    var visibility:Int = 1

    var transitionX:Float = 0.0f

    var transitionY:Float = 0.0f

    companion object{

        @JvmField
        val VISIBLE:Int = 1

        @JvmField
        val GONE:Int = 0
    }
}

fun execute(view:View, op:UiOp) = when(op){
    UiOp.Show -> view.visibility = View.VISIBLE
    UiOp.Hide -> view.visibility = View.GONE
    is UiOp.TransitionX -> view.transitionX = op.px
    is UiOp.TransitionY -> view.transitionY = op.px
}

class Ui(val uiOps: List<UiOp> = emptyList()){
    operator fun plus(uiOp:UiOp) = Ui(uiOps + uiOp)
}

var ui = Ui() +
         UiOp.Show +
         UiOp.TransitionX(20F) +
         UiOp.TransitionY(40F) +
         UiOp.Hide
fun run(view: View, ui: Ui){
    ui.uiOps.forEach{
        execute(view, it)
        println("run it ${it}")
    }
}

fun main(args:Array<String>){
    var ls:List<Int> = emptyList<Int>()
    println(ls)

    var ld = ls + 20
    println(ls)
    println(ld)

    var view = View()

    run(view, ui)
}