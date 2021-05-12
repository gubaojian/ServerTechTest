package com.efurture.com.efurture.algorithm.stock;

public class MaxStockSum {

    public static void main(String[] args) {
        int[] oneCases = {1, 0, 4, 2, 1};
        int[] twoCases = {1,9,6,9,1,7,1,1,5,9,9,9};
      

        System.out.println(searchStockMax(oneCases));
        System.out.println(searchStockMax(twoCases));
    }

    // 1 2 4 2
    // 1 0 4 2 1
    public static int searchStockMax(int[] prices){
        if(prices == null || prices.length <= 1){
            return 0;
        }
        int sum = 0;
        int maxSum = 0;
        for(int i=1; i<prices.length; i++){
            sum += prices[i] - prices[i-1];
            if(maxSum < sum){
                maxSum = sum;
            }
            if(sum < 0){
                sum = 0;
            }
        }
        return maxSum;
    }
}
