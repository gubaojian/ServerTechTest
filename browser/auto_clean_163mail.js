/***
 * 
 * 打开对应的文件夹，然后点开开发者选项，运行脚本
 */
// https://developer.mozilla.org/en-US/docs/Web/API/WakeLock


var wakeLock = null; 
function lockScreen() {
    if (navigator.wakeLock == null) {
        console.error('Your browser is not support WakeLock API!');
        return;
    }
    if (wakeLock) {
        return;
    }
    navigator.wakeLock.request('screen').then(result => {
        wakeLock = result;
        console.log('Wake Lock is actived!');
        wakeLock.addEventListener('release', () => {
            wakeLock = null;
            console.log('Wake Lock is released!');
        });
    }).catch((err) => {
        console.error(`Wake Lock is faild：${err.message}`);
    });
};
 


function autoDelete163Mail() {
    var dialogs = document.getElementsByClassName("js-component-msgbox");
    if (dialogs.length > 0) { //自动关闭错误提示
         var buttons = dialogs[0].getElementsByClassName("nui-btn-text");
         if (buttons.length > 0) {
            buttons[0].click();
            setTimeout(function(){ 
                autoDelete163Mail();
            }, 2000);
            return;
         }
    }
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
        }, 6500); //等待6.5秒删除下一页， 太短会弹出错误框， 无人看时候10秒比较保险
    }, 800)
}

setTimeout(function(){ 
    autoDelete163Mail();
}, 3000);



/***
 *  无人看管时，间隔长一些
 * 打开对应的文件夹，然后点开开发者选项，运行脚本
 *  无人看时候10秒比较保险, 删除到最后时加载比较慢
 */
var wakeLock = null; 
function lockScreen() {
    if (navigator.wakeLock  == null) {
        console.error('Your browser is not support WakeLock API!');
        return;
    }
    if (wakeLock) {
        return;
    }
    navigator.wakeLock.request('screen').then(result => {
        wakeLock = result;
        console.log('Wake Lock is actived!');
        wakeLock.addEventListener('release', () => {
            wakeLock = null;
            console.log('Wake Lock is released!');
        });
    }).catch((err) => {
        console.error(`Wake Lock is faild：${err.message}`);
    });
};
function autoDelete163Mail() {
    var dialogs = document.getElementsByClassName("js-component-msgbox");
    if (dialogs.length > 0) {
         var buttons = dialogs[0].getElementsByClassName("nui-btn-text");
         if (buttons.length > 0) {
            buttons[0].click();
            setTimeout(function(){ 
                autoDelete163Mail();
            }, 2000);
            return;
         }
    }
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
        }, 10*1000); //等待10秒删除下一页， 无人看时候10秒比较保险, 删除到最后时加载比较慢
    }, 800)
}

setTimeout(function(){ 
    autoDelete163Mail();
}, 3000);


/***
 *  无人看管时，间隔长一些
 * 打开对应的文件夹，然后点开开发者选项，运行脚本
 *  无人看时候10秒比较保险, 删除到最后时加载比较慢
 */
var wakeLock = null; 
function lockScreen() {
    if (navigator.wakeLock == null) {
        console.error('Your browser is not support WakeLock API!');
        return;
    }
    if (wakeLock) {
        return;
    }
    navigator.wakeLock.request('screen').then(result => {
        wakeLock = result;
        console.log('Wake Lock is actived!');
        wakeLock.addEventListener('release', () => {
            wakeLock = null;
            console.log('Wake Lock is released!');
        });
    }).catch((err) => {
        console.error(`Wake Lock is faild：${err.message}`);
    });
};

function autoDelete163Mail() {
    lockScreen();
    var dialogs = document.getElementsByClassName("js-component-msgbox");
    if (dialogs.length > 0) { //自动关闭错误提示
         var buttons = dialogs[0].getElementsByClassName("nui-btn-text");
         if (buttons.length > 0) {
            buttons[0].click();
            setTimeout(function(){ 
                autoDelete163Mail();
            }, 2000);
            return;
         }
    }

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
        }, 8*1000); //等待8秒删除下一页， 无人看时候10秒比较保险, 删除到最后时加载比较慢
    }, 800)
}

setTimeout(function(){ 
    autoDelete163Mail();
}, 3000);


// document.getElementsByClassName("nui-tabs-item-selected").length





var wakeLock = null; 
function lockScreen() {
    if (navigator.wakeLock == null) {
        console.error('Your browser is not support WakeLock API!');
        return;
    }
    if (wakeLock) {
        return;
    }
    navigator.wakeLock.request('screen').then(result => {
        wakeLock = result;
        console.log('Wake Lock is actived!');
        wakeLock.addEventListener('release', () => {
            wakeLock = null;
            console.log('Wake Lock is released!');
        });
    }).catch((err) => {
        console.error(`Wake Lock is faild：${err.message}`);
    });
};

function autoDelete163Mail() {
    lockScreen();
    var dialogs = document.getElementsByClassName("js-component-msgbox");
    if (dialogs.length > 0) { //自动关闭错误提示
         var buttons = dialogs[0].getElementsByClassName("nui-btn-text");
         if (buttons.length > 0) {
            buttons[0].click();
            setTimeout(function(){ 
                autoDelete163Mail();
            }, 2000);
            return;
         }
    }

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
        }, 7*1000); //等待8秒删除下一页， 无人看时候10秒比较保险, 删除到最后时加载比较慢
    }, 800)
}

setTimeout(function(){ 
    autoDelete163Mail();
}, 3000);
