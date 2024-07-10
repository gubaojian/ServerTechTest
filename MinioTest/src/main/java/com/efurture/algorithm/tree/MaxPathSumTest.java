package com.efurture.algorithm.tree;

public class MaxPathSumTest {


    public static void main(String[] args){
        TreeNode root = new TreeNode(1);
        addLeft(root, 2);
        TreeNode right =  addRight(root, 3);

        TreeNode next = addRight(right , 10);
        addLeft(right, 10);
        addRight(next, 100);

        System.out.println("maxPathSum " + maxPathSum(root));
        System.out.println("maxPathSum " + 100%3.0);
    }


    public static TreeNode addLeft(TreeNode node, int val){
        TreeNode leftNode = new TreeNode(val);
        node.left = leftNode;
        return leftNode;
    }

    public static TreeNode addRight(TreeNode node, int val){
        TreeNode rightNode = new TreeNode(val);
        node.right = rightNode;
        return rightNode;
    }



    public static int maxPathSum(TreeNode root){
        return new TreeMaxPathSum(root).getMaxPathSum();
    }



    private static class TreeMaxPathSum{
        private TreeNode root;
        private int maxPathSum;

        public TreeMaxPathSum(TreeNode root) {
            this.root = root;
            this.maxPathSum = Integer.MIN_VALUE;
        }

        public int getMaxPathSum(){
            maxPathSum(root);
            return maxPathSum;
        }

        private int maxPathSum(TreeNode root){
            int max = Integer.MIN_VALUE;
            int val  = root.val;
            int leftVal = 0;
            int rightVal = 0;
            max = Math.max(val, max);
            if(root.left != null){
                leftVal = maxPathSum(root.left);
                max = Math.max(max, leftVal);
                max = Math.max(max, val + leftVal);
            }
            if(root.right != null){
                rightVal = maxPathSum(root.right);
                max = Math.max(max, rightVal);
                max = Math.max(max, val + rightVal);
            }

            if(root.left != null && root.right != null){
                max = Math.max(max, val + leftVal + rightVal);
            }

            max =  Math.max(max, val);
            maxPathSum = Math.max(maxPathSum, max);



            int pathVal = Math.max(val, val + leftVal);
            pathVal = Math.max(val, val + rightVal);

            return pathVal;
        }
    }
}
