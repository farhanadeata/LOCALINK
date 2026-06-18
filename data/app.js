

let socket;

const username =
localStorage.getItem("username");

const role =
localStorage.getItem("role");

const chatContainer =
document.getElementById(
"chatContainer"
);

const messageInput =
document.getElementById(
"messageInput"
);

const sendBtn =
document.getElementById(
"sendBtn"
);

const onlineCount =
document.getElementById(
"onlineCount"
);

const clearBtn =
document.getElementById(
"clearBtn"
);

// ==========================
// CONNECT
// ==========================

function connectWS(){

    socket =
    new WebSocket(
    `ws://${location.host}/ws`
    );

    socket.onopen = ()=>{

        console.log(
        "WebSocket Connected"
        );

        showToast(
        "Terhubung"
        );
    };

    socket.onclose = ()=>{

        console.log(
        "WebSocket Closed"
        );

        showToast(
        "Reconnect..."
        );

        setTimeout(
        connectWS,
        2000
        );
    };

    socket.onerror = (err)=>{

        console.log(err);
    };

    socket.onmessage =
    (event)=>{

        const data =
        JSON.parse(
        event.data
        );

        handleServerData(
        data
        );
    };
}

// ==========================
// HANDLE SERVER
// ==========================

function handleServerData(data){

    switch(data.type){

        case "history":

            chatContainer.innerHTML="";

            data.messages.forEach(
            msg=>{
                renderMessage(msg);
            });

            scrollBottom();

        break;

        case "message":

            renderMessage(data);

            scrollBottom();

        break;

        case "online":

            onlineCount.innerText =
            "Online : " +
            data.count;

        break;

        case "clear":

            chatContainer.innerHTML =
            "";

            showToast(
            "Chat dibersihkan"
            );

        break;
    }
}

// ==========================
// SEND
// ==========================

function sendMessage(){

    const text =
    messageInput.value.trim();

    if(text === "")
        return;

    if(
      socket.readyState !==
      WebSocket.OPEN
    ){
        showToast(
        "Belum terhubung"
        );
        return;
    }

    socket.send(

      "SEND|"
      + username
      + "|"
      + role
      + "|"
      + text

    );

    messageInput.value="";
}

// ==========================
// RENDER
// ==========================

function renderMessage(msg){

    const mine =
    msg.username === username;

    const wrapper =
    document.createElement(
    "div"
    );

    wrapper.className =
    mine ?
    "message self" :
    "message";

    const roleClass =
    getRoleClass(
    msg.role
    );

    wrapper.innerHTML =

    `
    <div class="bubble">

      <div class="username">

        ${msg.username}

      </div>

      <div class="role ${roleClass}">

        ${msg.role}

      </div>

      <div class="chat-text">

        ${escapeHtml(
        msg.message
        )}

      </div>

      <div class="time">

        ${formatTime(
        msg.time
        )}

      </div>

    </div>
    `;

    chatContainer.appendChild(
    wrapper
    );
}

// ==========================
// ROLE COLOR
// ==========================

function getRoleClass(role){

    switch(role){

        case "Admin":
            return "admin";

        case "Petugas":
            return "petugas";

        case "Operator":
            return "operator";

        default:
            return "tamu";
    }
}

// ==========================
// FORMAT TIME
// ==========================

function formatTime(ts){

    const date =
    new Date(
      Number(ts) * 1000
    );

    return date
    .toLocaleTimeString(
      "id-ID",
      {
        hour:"2-digit",
        minute:"2-digit"
      }
    );
}

// ==========================
// ESCAPE
// ==========================

function escapeHtml(text){

    const div =
    document.createElement(
    "div"
    );

    div.innerText =
    text;

    return div.innerHTML;
}

// ==========================
// SCROLL
// ==========================

function scrollBottom(){

    chatContainer.scrollTop =
    chatContainer.scrollHeight;
}

// ==========================
// SEND BUTTON
// ==========================

sendBtn.addEventListener(
"click",
sendMessage
);

// ==========================
// ENTER KEY
// ==========================

messageInput.addEventListener(
"keydown",
(e)=>{

    if(
      e.key === "Enter"
    ){

        sendMessage();
    }
});

// ==========================
// CLEAR CHAT
// ==========================

if(clearBtn){

    clearBtn.onclick =
    ()=>{

        if(
        role !== "Admin"
        ){

            showToast(
            "Khusus Admin"
            );

            return;
        }

        if(
        confirm(
        "Hapus semua chat?"
        )
        ){

            socket.send(
            "CLEAR"
            );
        }
    };
}

// ==========================
// START
// ==========================

connectWS();