package hello

class TestSubject{

    fun method(){

    }
}

public class MyTest {
    lateinit var subject:TestSubject

    @SetUp fun setup(){
        subject = TestSubject()
    }

    @Test fun test(){
        subject.method()
    }

}