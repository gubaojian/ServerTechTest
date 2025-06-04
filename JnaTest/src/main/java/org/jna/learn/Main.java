package org.jna.learn;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;


public class Main {

    public interface CLibrary extends Library {
        CLibrary INSTANCE = (CLibrary)
                Native.load((Platform.isWindows() ? "msvcrt" : "c"),
                        CLibrary.class);

        void printf(String format, Object... args);
        int malloc_trim(int size);
    }

    public static void main(String[] args) {
        System.out.println("Hello world!");
        CLibrary.INSTANCE.printf("hello world2");
        CLibrary.INSTANCE.malloc_trim(0);
    }
}