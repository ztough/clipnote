(function () {
    'use strict';
    let socket;
    const socketUrl = 'ws://127.0.0.1:5488';
    var ws = false;
    var video = null;
    var cnt = getQueryVariable("cnt");
    var cnf = getQueryVariable("cnf");
    var _videoObj = [];
    var _videoSrc = [];
    function hackAttachShadow() {
        if (window._hasHackAttachShadow_) return;
        try {
            window.Element.prototype._attachShadow =
                window.Element.prototype.attachShadow;
            window.Element.prototype.attachShadow = function () {
                const arg = arguments;
                if (arg[0] && arg[0].mode) {
                    arg[0].mode = "open";
                }
                const shadowRoot = this._attachShadow.apply(this, arg);
                shadowRoot.host.setAttribute("clipnote", "");
                return shadowRoot;
            };
            window._hasHackAttachShadow_ = true;
        } catch (e) {
        }
    }

    setInterval(function () {
        if (video && socket && !video.paused) {
            if (socket.readyState == 1) {
                var obj = { action: "info", duration: parseInt(video.duration), current: parseInt(video.currentTime) };
                socket.send(JSON.stringify(obj));
            }
        }

    }, 500);
    var observer = new IntersectionObserver(function (entries) {
        entries.forEach(function (entry) {
            if (entry.isIntersecting) {
                if (href().includes("aliyundrive.com")) {
                    var play = document.getElementsByClassName("btn--UrTVT");
                    play[1].click();
                } else {
                    if (video) {
                        video.play();
                    }

                }

            }
        });
    }, { threshold: 0 });
    updete_video();
    function updete_video() {
        hackAttachShadow();
        let videoObj = [];
        let videoSrc = [];
        document.querySelectorAll("video ,audio").forEach(function (video) {
            if (video.currentSrc != "" && video.currentSrc != undefined) {
                videoObj.push(video);
                videoSrc.push(video.currentSrc);
            }
        });
        document.querySelectorAll("iframe").forEach(function (iframe) {
            if (iframe.contentDocument == null) { return; }
            iframe.contentDocument.querySelectorAll("video ,audio").forEach(function (video) {
                if (video.currentSrc != "" && video.currentSrc != undefined) {
                    videoObj.push(video);
                    videoSrc.push(video.currentSrc);
                }
            });
        });
        document.querySelectorAll("[clipnote]").forEach(function (elem) {
            elem.shadowRoot.querySelectorAll("video ,audio").forEach(function (video) {
                if (video.currentSrc != "" && video.currentSrc != undefined) {
                    videoObj.push(video);
                    videoSrc.push(video.currentSrc);
                }
            });
        });
        if (videoObj.length > 0) {
            if (videoObj.length !== _videoObj.length || videoSrc.toString() !== _videoSrc.toString()) {
                _videoSrc = videoSrc;
                _videoObj = videoObj;

                if (ws == false) {
                    seek();
                    createWebSocket();
                }
                _videoObj.forEach(function (videoElement) {

                    if (!href().includes("cloud.189.cn")) {
                        videoElement.setAttribute("crossOrigin", 'anonymous');
                    }
                    videoElement.addEventListener("timeupdate", function () {
                        if (cnt) {
                            if (cnt.includes("-")) {
                                if ((video.currentTime >= cnt.split("-")[1]) && (parseInt(video.currentTime) == parseInt(cnt.split("-")[1]))) {
                                    video.pause();
                                    cnt = "";
                                }
                            }
                            if (cnt.includes("~")) {
                                if ((video.currentTime >= cnt.split("~")[1]) && (parseInt(video.currentTime) == parseInt(cnt.split("~")[1]))) {
                                    video.currentTime = cnt.split("~")[0];
                                }
                            }
                        }

                    });
                    observer.observe(videoElement);
                });
            }
        }
        requestAnimationFrame(updete_video);
    }

    var duration = 0;
    var v = null;
    bind_video();
    function bind_video() {
        _videoObj.forEach(function (value) {
            if (value.duration >= duration) {
                duration = value.duration;
                v = value;
            }
        });
        if (v && (v != video || video.src == "")) {
            video = v;
            seek();
            duration = 0;
            v = null;
        }
        requestAnimationFrame(bind_video);
    }
    function addParameterToUrl(url, paramName, paramValue) {
        const separator = url.includes('?') ? '&' : '?';
        const newUrl = `${url}${separator}${paramName}=${paramValue}`;
        return newUrl;
    }
    function nocnf(url) {
        let link = url;
        var regex = /[/?&]cnf=.*/g;
        var match = regex.exec(link);
        if (match) {
            link = link.replace(match[0], "");
        }
        return link;
    }
    function nocnt(url) {
        let link = url;
        var regex = /[/?&]cnt=.*/g;
        var match = regex.exec(link);
        if (match) {
            link = link.replace(match[0], "");
        }
        return link;
    }
    function nohash(url) {
        let link = url;
        var regex = /#.*$/g;
        var match = regex.exec(link);
        if (match) {
            link = link.replace(match[0], "");
        }
        return link;
    }

    function href() {
        var url = nohash(nocnt(nocnf(decodeURIComponent(window.location.href))));
        if (filename() != "") { }
        if (url.includes("aliyundrive.com")) {
            url = addParameterToUrl(url, "cnf", filename())
        }
        url += window.location.hash;
        return url;
    }
    function filename() {
        if (window.location.href.includes("aliyundrive.com")) {
            var filename = document.querySelector(".text--KBVB3");
            if (filename) {
                var f = filename.textContent;
                return f;
            }
        }
        if (window.location.href.includes("cloud.189.cn")) {
            var filename = document.querySelector(".video-title");
            if (filename) {
                var f = filename.textContent;
                return f;
            }
        }
        if (window.location.href.includes("pan.xunlei")) {
            var filename = document.querySelector(".video-name");
            if (filename) {
                var f = filename.getAttribute("title");
                return f;
            }
        }
        return "";
    }
    document.onreadystatechange = function () {
        if (document.readyState === 'complete') {
            click();
        }
    };
    function pause() { if (video) { video.pause(); } }
    function play() {
        if (video) {
            video.muted = false;
            video.play();
        }
    }
    function getQueryVariable(variable) {
        var query = decodeURIComponent(window.location.search.substring(1));
        var vars = query.split("&");
        for (var i = 0; i < vars.length; i++) {
            var pair = vars[i].split("=");
            if (pair[0] == variable) { return pair[1]; }
        }
        return "";
    }

    function aliyundrive() {
        var elements = document.getElementsByClassName('title--HvI83');
        if (elements.length > 0) {
            for (var i = 0; i < elements.length; i++) {
                if (elements[i].textContent == cnf) {
                    elements[i].click();
                    play();
                }
            }
        }
    }
    function aliyundrive2() {
        var elements = document.getElementsByClassName('text-primary--JzAb9');
        if (elements.length > 0) {
            for (var i = 0; i < elements.length; i++) {
                if (elements[i].textContent == cnf) {
                    elements[i].click();
                    play();
                }
            }
        }
    }
    function tianyi() {
        var elements = document.getElementsByClassName('file-item-name-fileName-span');
        var ad = document.getElementsByClassName("advertising-popup-close");
        if (ad[0]) { ad[0].click(); }
        if (elements.length > 0 && elements[0].getAttribute("title")) {
            for (var i = 0; i < elements.length; i++) {

                if ((elements[i].getAttribute("title") == cnf)) {
                    var previousSibling = elements[i].parentNode.parentNode.previousElementSibling;
                    setTimeout(function () { previousSibling.click(); }, 1500);
                    play();
                }
            }
        }
    }
    function xunlei() {
        var elements = document.getElementsByTagName('a');
        if (elements.length > 0) {
            for (var i = 0; i < elements.length; i++) {
                if (elements[i].getAttribute("title") == cnf) {
                    elements[i].click();
                    play();
                }
            }
        }
    }
    function click() {
        if (href().includes("weibo.com") || href().includes("weibo.cn")) {
            var button = document.querySelector('.mwbv-play-button');
            if (button) { button.click(); }
        }
        if (href().includes("aliyundrive.com")) {

            setTimeout(aliyundrive, 1500);
            setTimeout(aliyundrive2, 1500);
        }
        if (href().includes("cloud.189.cn")) {
            setTimeout(tianyi, 1500);
        }
        if (href().includes("mp.weixin")) {
            var play = document.querySelector('.mid_play_box.reset_btn');
            play.click();
        }
        if (href().includes("pan.xunlei")) {
            setTimeout(xunlei, 1500);
        }
        if (href().includes("zhihu.com")) {
            var play = document.getElementsByClassName("_1smmk5k");
            play[0].click();
        }
    }
    function seek() {
        var pos = -2;
        if (cnt) {
            if (cnt.includes("-")) {
                pos = cnt.split("-")[0];
            }
            else if (cnt.includes("~")) {
                pos = cnt.split("~")[0];
            } else if (cnt != "") {
                pos = cnt;
            }
        }

        if (video && pos >= 0) {
            setTimeout(function () {
                video.currentTime = pos;
                if (parseInt(video.currentTime) != parseInt(pos)) {
                    requestAnimationFrame(seek);
                } else {
                    play();
                }
            }, 500);
        } else {
            requestAnimationFrame(seek);
        }

    }
    document.addEventListener('visibilitychange', function () {
        if (document.visibilityState == 'visible') {
            play();
            if (_videoObj.length > 0 && socket.readyState != 1) {
                createWebSocket();
            }
        } else {
            pause();
            if (_videoObj.length > 0) {
                socket.close();
            }
        }
    });

    function createWebSocket() {
        ws = true;
        socket = new WebSocket(socketUrl);
        socket.addEventListener('open', function (event) {
            play();
        });
        socket.addEventListener('message', function (event) {
            var json = JSON.parse(event.data);
            if (document.visibilityState != 'visible') { pause(); return; }
            if (json["action"] == "pos") {
                var currentTime = video.currentTime;
                var obj = { action: "pos", data: currentTime, url: href() };
                socket.send(JSON.stringify(obj));
            }
            if (json["action"] == "pos") {
                cnt = json["cnt"];
                seek()
            }
            if (json["action"] == "seek") {
                var url = new URL(href());
                var domain = url.host;
                if ((href().includes(nocnf(json["url"])) || nocnf(json["url"]).includes(href())) && domain != href().replace("https://", "").replace("http://", "").replace("/", "")) {
                    cnt = json["cnt"];
                    seek()
                } else {

                    window.location.href = json["data"];
                }

            }
            if (json["action"] == "playorpause") {
                if (video.paused) {
                    play();
                } else {
                    pause();
                }
            }
            if (json["action"] == "pause") {
                pause();
            }
            if (json["action"] == "play") {
                play()
            }

            if (json["action"] == "start") {
                var obj = { action: "start", duration: parseInt(video.duration) };
                socket.send(JSON.stringify(obj));
            }
            if (json["action"] == "seekandsnapshots") {
                pause();
                var canvas = document.createElement('canvas');
                canvas.style.cssText = 'display: none;';
                document.body.appendChild(canvas);
                canvas.width = video.videoWidth;
                canvas.height = video.videoHeight;
                var context = canvas.getContext('2d');
                var interval = json["interval"];
                var start = json["start"];
                var end = json["end"];
                var arr = json["arr"];
                var mode = json["mode"];
                var img = json["img"];
                var url=href();
                var duration=parseInt(video.duration);
                if (end > duration) {
                    end = duration;
                }

                async function captureFrames() {
                    if(arr){

                        for(let item of arr){
                            if(typeof item==="number"){
                                await seekAndCapture(item);
                            }else{
                                var obj = { action: "markdown",text:item};
                                socket.send(JSON.stringify(obj));
                            }
                        }

                    }else{
                        
                        for (var i = start; i <= end; i += interval) {
                            await seekAndCapture(i);
                        }

                    }
                    var obj = { action: "finish"};
                    socket.send(JSON.stringify(obj));
                   
                }
                
                function seekAndCapture(targetTime) {
                    return new Promise((resolve, reject) => {
                        if((mode==0&&(img==0||img==1||img==2))||(mode==1&&(img==0||img==1))){
                            video.currentTime = targetTime;
                            function onTimeUpdate() {
                                if (Math.abs(video.currentTime - targetTime) < 0.01) {
                                    video.removeEventListener('timeupdate', onTimeUpdate);
                                    context.drawImage(video, 0, 0, canvas.width, canvas.height);
                                    try {
                                        var screenshotDataUrl = canvas.toDataURL('image/png');
                                        var obj = { action: "markdown", data: screenshotDataUrl,duration:duration, time: targetTime,mode:mode,url:url,img:img };
                                        socket.send(JSON.stringify(obj));
                                        resolve();
                                    } catch (err) {
                                        var obj = { action: "markdown", data: "",duration:duration, time: targetTime,mode:mode ,url:url,img:img};
                                        socket.send(JSON.stringify(obj));
                                        reject(err);
                                    }
                                }
                            }
                            video.addEventListener('timeupdate', onTimeUpdate);
                        }else{
                            var obj = { action: "markdown", data: "",duration:duration, time: targetTime,mode:mode,url:url,img:img};socket.send(JSON.stringify(obj));
                            resolve();
                        }
                      
                    });
                }
                captureFrames();
            }

            if (json["action"] == "snapshots") {
                var canvas = document.createElement('canvas');
                canvas.style.cssText = 'display: none;';
                document.body.appendChild(canvas);
                canvas.width = video.videoWidth;
                canvas.height = video.videoHeight;
                var context = canvas.getContext('2d');
                context.drawImage(video, 0, 0, canvas.width, canvas.height);
                try {
                    var screenshotDataUrl = canvas.toDataURL('image/png');
                    var obj = { action: "snapshots", data: screenshotDataUrl, url: href() };
                    socket.send(JSON.stringify(obj));
                } catch {
                    alert("服务器跨域限制,请手动操作！");
                    canvas.style = 'max-width:100%';
                    const previewPage = window.open('', '_blank');
                    if (previewPage && previewPage.document) {
                        previewPage.document.title = 'snapshots';
                        previewPage.document.body.style.textAlign = 'center';
                        previewPage.document.body.style.background = '#000';
                        previewPage.document.body.appendChild(canvas);
                    } else {
                        alert("无法打开窗口，请设置浏览器权限（设置->cookie和网站权限->弹出窗口和重定向->允许）");
                    }
                    return;
                }
            }
            if (json["action"] == "close") {
                pause();
                window.close();
            }
            if (json["action"] == "kj") {
                video.currentTime = video.currentTime + 5;
            }
            if (json["action"] == "kt") {
                video.currentTime = video.currentTime - 5;
            }
            if (json["action"] == "jiasu") {
                video.playbackRate = video.playbackRate + 0.25;
            }
            if (json["action"] == "jiansu") {
                video.playbackRate = video.playbackRate - 0.25;
            }
            if (json["action"] == "changsu") {
                video.playbackRate = 1.0;
            }

            if (json["action"] == "yljia") {
                if (video.volume + 0.1 > 1.0) {
                    video.volume = 1.0;
                } else {
                    video.volume = video.volume + 0.1;
                }
            }
            if (json["action"] == "yljian") {

                if (video.volume - 0.1 < 0.0) {
                    video.volume = 0.0;
                } else {
                    video.volume = video.volume - 0.1;
                }
            }
        });
        socket.addEventListener('close', function (event) {
            if (document.visibilityState == "visible" && socket.readyState != 1) {
                setTimeout(createWebSocket, 500);
            }
        });
    }
})();