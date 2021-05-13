package com.efurture.algorithm.tree;

public class RootMaxPathTest {


    public static void main(String[] args){
        TreeNode root = new TreeNode(1);
        addLeft(root, 2);
        TreeNode right =  addRight(root, 3);

        TreeNode next = addRight(right , 10);
        addLeft(right, 10);
        addRight(next, -100);

        System.out.println("maxPathSum " + maxPathSum(root));

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
        return new RootMaxPathSum(root).getMaxPathSum();
    }



    private static class RootMaxPathSum {
        private TreeNode root;
        private int maxPathSum;

        public RootMaxPathSum(TreeNode root) {
            this.root = root;
            this.maxPathSum = Integer.MIN_VALUE;
        }

        public int getMaxPathSum(){
            maxPathSum(root, 0);
            return maxPathSum;
        }

        private void maxPathSum(TreeNode root, int parentVal){
            int max = Integer.MIN_VALUE;
            int val = root.val + parentVal;
            max = Math.max(val, max);
            maxPathSum = Math.max(max, maxPathSum);
            if(root.left != null){
                maxPathSum(root.left, val);
            }
            if(root.right != null){
                maxPathSum(root.right, val);
            }
        }
    }
}
