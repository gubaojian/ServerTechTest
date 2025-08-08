package jdkx.util.compat;

public class Compat {

    public static  class Math {

        public static int addExact(int x, int y) {
            int r = x + y;
            // HD 2-12 Overflow iff both arguments have the opposite sign of the result
            if (((x ^ r) & (y ^ r)) < 0) {
                throw new ArithmeticException("integer overflow");
            }
            return r;
        }

        public static long addExact(long x, long y) {
            long r = x + y;
            // HD 2-12 Overflow iff both arguments have the opposite sign of the result
            if (((x ^ r) & (y ^ r)) < 0) {
                throw new ArithmeticException("long overflow");
            }
            return r;
        }

        public static int multiplyExact(int x, int y) {
            long r = (long)x * (long)y;
            if ((int)r != r) {
                throw new ArithmeticException("integer overflow");
            }
            return (int)r;
        }
    }

    public static  class Objects {
        public static int checkFromIndexSize(int off, int len, int length) {
            if (off < 0 || len < 0 || (long) off + len > length) {
                throw new IndexOutOfBoundsException("Range [off=" + off + ", len=" + len + "] out of bounds for length " + length);
            }
            return off;
        }
    }

}
