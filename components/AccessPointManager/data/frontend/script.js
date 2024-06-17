let AccessoryJSON = [];
let AccessoryTypeArray = {
    "LIGHT": ["name", "lightPin", "buttonPin"],
    "FAN": ["name", "fanPin", "buttonPin"],
    "OUTLET": ["name", "outletPin", "buttonPin"],
    "SWITCH": ["name", "switchPin", "buttonPin"],
    "BUTTON": ["name", "buttonPin"],
    "BOILER": ["name", "boilerPin", "buttonPin"],
    "DOOR": ["name", "doorPin", "doorButtonPin", "doorbellButtonPin", "openDuration"],
    "WINDOW": ["name", "motorUpPin", "motorDownPin", "buttonUpPin", "buttonDownPin", "timeToOpen", "timeToClose"],
    "BLIND": ["name", "motorUpPin", "motorDownPin", "buttonUpPin", "buttonDownPin", "timeToOpen", "timeToClose"]
};

async function onLoadFunc() {
    getAccsoriesFromDB().then(function () { gridLoader(); });
    getStoredWifi();
}

function SideBarClick(clickedElement) {
    ////////////////// change sidebar //////////////////
    let current = document.getElementsByClassName("sidebarActive");
    current[0].className = current[0].className.replace("sidebarActive", "");
    clickedElement.className += " sidebarActive";
    ////////////////// change page //////////////////
    let currentBody = document.getElementsByClassName("bodyActive");
    currentBody[0].className = currentBody[0].className.replace("bodyActive", "bodyHidden");
    let pageRequest = clickedElement.getAttribute("data-page");
    let newPage = document.getElementById(pageRequest);
    newPage.className = newPage.className.replace("bodyHidden", "bodyActive");
}

async function getRequest(url) {
    try {
        const res = await fetch(url);
        if (res.ok) {
            return await res.json();
        } else {
            const error = await res.text();
            throw new Error(error);
        }
    } catch (e) {
        console.error('Failed to send get request', e);
    }
}

async function putRequest(url, body) {
    try {
        const res = await fetch(url, { method: 'PUT', body: JSON.stringify(body) });
        if (res.ok) {
            return await res.json();
        } else {
            const error = await res.text();
            throw new Error(error);
        }
    } catch (e) {
        console.error('Failed to send put request', e);
    }
}

async function postRequest(url, body) {
    try {
        const res = await fetch(url, { method: 'POST', body: JSON.stringify(body) });
        if (res.ok) {
            return await res.json();
        } else {
            const error = await res.text();
            throw new Error(error);
        }
    } catch (e) {
        console.error('Failed to send post request', e);
    }
}

async function getStoredWifi() {
    try {
        getRequest("/wifi/stored").then(function onSuccess(responseData) {
            if (responseData) {
                document.getElementById("WifiSSID").value = responseData.data.SSID;
                document.getElementById("WifiPassword").value = responseData.data.PASSWORD;
            }
        });
    } catch { }
}

async function saveWifi() {
    let SSID = document.getElementById("WifiSSID").value;
    let PASSWORD = document.getElementById("WifiPassword").value;
    let response = {
        "SSID": SSID,
        "PASSWORD": PASSWORD
    };
    postRequest("/wifi/save", response);
}

async function tryWifiPasswordConnect() {
    document.getElementById("loaderConnect").style.display = "inline-block";
    try {
        let SSID = document.getElementById("WifiSSID").value;
        let PASSWORD = document.getElementById("WifiPassword").value;
        let response = {
            "SSID": SSID,
            "PASSWORD": PASSWORD
        };
        putRequest("/wifi/connect", response).then(function onSuccess(responseData) {
            if (responseData.data) {
                document.getElementById("TryConnect").style.color = "green";
                document.getElementById("TryConnect").innerHTML = "Good";
            } else {
                document.getElementById("TryConnect").style.color = "red";
                document.getElementById("TryConnect").innerHTML = "Wrong";
            }
            document.getElementById("loaderConnect").style.display = "none";
        });
    } catch { () => { document.getElementById("loaderConnect").style.display = "none"; } }
}

function gridLoader() {
    let gridViewContainer = document.getElementById("GridContainer");
    gridViewContainer.innerHTML = "";

    for (let i = 0; i < AccessoryJSON.length; i++) {
        gridViewContainer.appendChild(creatGridItem(AccessoryJSON[i], i));
    }
    onWindowResizeModifyGridSize();
    window.onresize = onWindowResizeModifyGridSize;
}

function creatGridItem(itemData, indexJson) {
    let tableData = document.createElement("table");
    let tableDiv = document.createElement("div");
    tableDiv.appendChild(tableData);
    let imgIcon = document.createElement("img");
    let imgDiv = document.createElement("div");
    imgDiv.appendChild(imgIcon);
    let newItem = document.createElement("div");
    newItem.appendChild(imgDiv);
    newItem.appendChild(tableDiv);
    imgIcon.src = "iconlight.png";
    newItem.className = "grid-item";
    newItem.style.width = 300;
    Object.keys(itemData).forEach(function (key) {
        // if (key != "type")
        tableData.appendChild(creatPropertyRow(key, itemData[key]));
    });

    let buttonEdit = document.createElement("img");
    buttonEdit.src = "editicon.png";
    buttonEdit.className = "imgButton";
    buttonEdit.onclick = function () { editeButton(indexJson); };
    imgDiv.appendChild(buttonEdit);

    let buttonRemove = document.createElement("img");
    buttonRemove.src = "removeicon.png";
    buttonRemove.className = "imgButton";
    buttonRemove.onclick = function () { removeButton(indexJson); };
    imgDiv.appendChild(buttonRemove);

    return newItem;
}

function creatPropertyRow(Key, value) {
    let tr = document.createElement("tr");
    let tdKey = document.createElement("td");
    let tdValue = document.createElement("td");
    tr.appendChild(tdKey);
    tr.appendChild(tdValue);
    let lableKey = document.createElement("lable");
    let lableValue = document.createElement("lable");
    tdKey.appendChild(lableKey);
    tdValue.appendChild(lableValue);
    lableKey.textContent = Key + " :";
    lableValue.textContent = value;
    return tr;
}

function onWindowResizeModifyGridSize() {
    if (document.getElementsByClassName("grid-item").length < 1)
        return;
    let gridViewContainer = document.getElementById("GridContainer");
    let numberOfItems = document.getElementsByClassName("grid-item").length;
    let bodyWidth = (document.getElementById("rightBody").clientWidth * 0.9) - 20 - 20;
    let itemWidth = parseInt(document.getElementsByClassName("grid-item")[0].style.width, 10);

    let maxItemInRow = ~~(bodyWidth / (itemWidth + 20 + 10));
    let autoApeend = "";

    for (let i = 0; i < maxItemInRow && i < numberOfItems; i++) {
        autoApeend += "auto ";
    }

    gridViewContainer.style.gridTemplateColumns = autoApeend;


}

async function getAccsoriesFromDB() {
    await getRequest("/accessories/stored").then(function onSuccess(responseData) {
        if (responseData && responseData.message == "success") {
            AccessoryJSON = responseData.data;
        }
    });
};

function editeButton(indexJson) {
    var modal = document.getElementById("editFullScreen");
    modal.style.display = "block";
    let gridViewContainer = document.getElementById("editTableView");
    gridViewContainer.innerHTML = "";
    let itemData = AccessoryJSON[indexJson];
    let tableEdit = document.createElement("table");
    gridViewContainer.appendChild(tableEdit);

    Object.keys(itemData).forEach(function (key) {
        if (key != "type" && key != "aidMeta")
            tableEdit.appendChild(creatPropertyRowEdit(key, itemData[key], indexJson));
    });
}

function removeButton(indexJson) {
    AccessoryJSON.splice(indexJson, 1);
    gridLoader();
}

function creatPropertyRowEdit(Key, value, indexJson) {
    let tr = document.createElement("tr");
    let tdKey = document.createElement("td");
    let tdValue = document.createElement("td");
    tr.appendChild(tdKey);
    tr.appendChild(tdValue);
    let lableKey = document.createElement("lable");
    let lableValue = document.createElement("input");
    tdKey.appendChild(lableKey);
    tdValue.appendChild(lableValue);
    lableKey.textContent = Key + " :";
    lableValue.value = value;
    lableValue.onchange = function () { saveNewValueJson(Key, lableValue, indexJson); };
    if (Key == "name")
        lableValue.maxLength = 50;
    else if (Key.includes("Pin")) {
        lableValue.type = "number";
        lableValue.min = 1;
        lableValue.max = 16;
        lableValue.step = 1;
    } else lableValue.type = "number";
    return tr;
}

function saveNewValueJson(Key, newValue, indexJson) {
    if (newValue.value == "") {
        alert("The " + Key + " field is Required.");
        newValue.value = AccessoryJSON[indexJson][Key];
        return false;
    }
    if (Key == "name") {
        if (AccessoryJSON.some(x => x.name === newValue.value)) {
            alert("The " + Key + " field should be uniqe.");
            newValue.value = AccessoryJSON[indexJson][Key];
            return false;
        }
    }
    AccessoryJSON[indexJson][Key] = newValue.value.trim();
    newValue.value = AccessoryJSON[indexJson][Key];
}

function closeButton() {
    document.getElementById("editFullScreen").style.display = "none";
    gridLoader();
}

function addButton() {
    var modal = document.getElementById("addAccessoryScreen");
    modal.style.display = "block";
    let gridViewContainer = document.getElementById("addTableView");
    gridViewContainer.innerHTML = "";
    let tableEdit = document.createElement("table");
    gridViewContainer.appendChild(tableEdit);
    let trDiv = document.createElement("tr");
    let tdDivKey = document.createElement("td");
    let tdDivValue = document.createElement("td");
    trDiv.appendChild(tdDivKey);
    trDiv.appendChild(tdDivValue);
    tableEdit.appendChild(trDiv);
    tdDivKey.innerHTML = "Accessory type: ";
    let selectOptions = document.createElement("select");
    selectOptions.id = "AccType";
    Object.keys(AccessoryTypeArray).forEach(function (key) {
        let myOption = document.createElement("option");
        myOption.value = key;
        myOption.innerHTML = key;
        selectOptions.appendChild(myOption);
    });
    tdDivValue.appendChild(selectOptions);
}

function addButtonDone() {

    let accessoryType = document.getElementById("AccType").value
    console.log(accessoryType);
    let accessProp = AccessoryTypeArray[accessoryType];
    let newAccessory = { "type": accessoryType };
    newAccessory["aidMeta"] = "" + Math.floor(Math.random() * 1000000) + "";
    let newName = accessoryType;
    let k = 1;
    for (let i = 0; i < accessProp.length; i++) {
        if (accessProp[i] == "name") {
            while (AccessoryJSON.some(x => x.name === (newName + " " + k))) {
                k++;
            }
            newAccessory[accessProp[i]] = newName + " " + k;
        }
        else
            newAccessory[accessProp[i]] = 1;
    }
    AccessoryJSON.push(newAccessory);

    document.getElementById("addAccessoryScreen").style.display = "none";
    gridLoader();
    editeButton(AccessoryJSON.length - 1);
}

async function sendAccessoriesToServer() {
    await postRequest("/accessories/save", AccessoryJSON);
    location.reload();
}

function unpairDevice() {
    if (confirm("Are you sure you want to Unpair this decive, you will not be able to control it throw your IOS devices until you pair it again!, the device will restart after unpairing.") == true) {
        getRequest("/command/unpair");
    }
}

function factoryresetDevice() {
    if (confirm("Are you sure you want to factory reset the device, all data will be erased.") == true) {
        getRequest("/command/factory");
    }
}

function updatefirmwareDevice() {
    if (confirm("Are you sure you want to factory reset the device, all data will be erased.") == true) {
        getRequest("/command/updatefirmware").then(function onSuccess(responseData) {
            document.getElementById("updateLabel").innerHTML = responseData.message;
        });
    }
}

function existDevice() {
    if (confirm("Are you sure you want to exist program mode, it will restart your device.") == true) {
        getRequest("/command/restart");
    }
}