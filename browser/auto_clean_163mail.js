/***
 * 
 * 打开对应的文件夹，然后点开开发者选项，运行脚本
 */

function autoDelete163Mail() {
    var headers = document.getElementsByClassName("frame-main-cont-head");
    var target = null;
    for(var i=0; i<headers.length; i++) {
        if (headers[i].getAttribute("tabindex") == "0") {
            target = headers[i]; //最后一个是顶部菜单
        }
        console.log(headers[i]);
    }
    
    console.log(target);
    
    var buttons = target.getElementsByClassName("nui-toolbar-item");
    
    //选中邮件
    console.log(buttons[0]);
    buttons[0].getElementsByClassName("js-component-checkbox-dc")[0].click();
    
    console.log(buttons[1]);
    
    setTimeout(function() {
        //点击删除按钮删除
        buttons[1].getElementsByClassName("js-component-button")[0].click();
        console.log("auto delete mail success");
        setTimeout(function(){ 
            autoDelete163Mail();
        }, 6500); //等待6秒删除下一页， 无人看时候10秒比较保险
    }, 500)
}

setTimeout(function(){ 
    autoDelete163Mail();
}, 3000);



/***
 *  无人看管时，间隔长一些
 * 打开对应的文件夹，然后点开开发者选项，运行脚本
 */
function autoDelete163Mail() {
    var headers = document.getElementsByClassName("frame-main-cont-head");
    var target = null;
    for(var i=0; i<headers.length; i++) {
        if (headers[i].getAttribute("tabindex") == "0") {
            target = headers[i]; //最后一个是顶部菜单
        }
        console.log(headers[i]);
    }
    
    console.log(target);
    
    var buttons = target.getElementsByClassName("nui-toolbar-item");
    
    //选中邮件
    console.log(buttons[0]);
    buttons[0].getElementsByClassName("js-component-checkbox-dc")[0].click();
    
    console.log(buttons[1]);
    
    setTimeout(function() {
        //点击删除按钮删除
        buttons[1].getElementsByClassName("js-component-button")[0].click();
        console.log("auto delete mail success");
        setTimeout(function(){ 
            autoDelete163Mail();
        }, 1000); //等待10秒删除下一页， 无人看时候10秒比较保险
    }, 500)
}

setTimeout(function(){ 
    autoDelete163Mail();
}, 3000);

