/* 
 * 
 * WebSpec (TF2)
 *
 */


var TFClass = {'Scout': 1, 'Sniper': 2, 'Soldier': 3, 'Demoman': 4, 'Medic': 5, 'Heavy': 6, 'Pyro': 7, 'Spy': 8, 'Engineer': 9};
var WebSpec = {};

var dbg_framerate = 20;
var dbg_resizeKeepZoom = true;

function init() {
	WebSpec.socket = null;
	WebSpec.canvas = null;
	WebSpec.host = "ws://localhost:28020/webspec";
	WebSpec.screenCanvas = null;
	WebSpec.ctx = null;
	WebSpec.screenCTX = null;
	WebSpec.background = null;
	WebSpec.map = null;
	WebSpec.serverName = null;
	WebSpec.team = ["Unassigned", "Spectator", "RED", "BLU"];
	WebSpec.teamReadyState = [0, 0];
	WebSpec.teamPoints = [0, 5];
	WebSpec.teamPlayersAmount = [0, 0, 0];
	WebSpec.teamPlayersAlive = [0, 0];
	WebSpec.players = new Array();
	WebSpec.mapSettingsLoaded = false;
	WebSpec.mapSettingsFailed = false;
	WebSpec.mapSettings = {};
	WebSpec.scaling = 1.0;
	WebSpec.playerRadius = 3;
	WebSpec.width = window.innerWidth;
	WebSpec.height = window.innerHeight;
	WebSpec.timer = null;
	WebSpec.roundEnded = -1;
	WebSpec.roundEndTime = -1;
	WebSpec.roundStartTime = -1;
	WebSpec.mp_roundtime = -1;
	WebSpec.killFeed = new Array();
	WebSpec.killFeedFadeTime = 5;
	WebSpec.chat = new Array();
	WebSpec.chatHoldTime = 20;
	WebSpec.chatFadeTime = 10;
	WebSpec.totalUsersWatching = 0;
	WebSpec.classImages = new Array(null, new Image(), new Image(), new Image, new Image(), new Image(), new Image, new Image(), new Image(), new Image);
	WebSpec._connected = false;
	WebSpec._disconnected = false;
	WebSpec._disconnectCode = "";
	WebSpec._disconnectRS = "";
	WebSpec._disconnectReason = "";
	WebSpec._disconnectClean = "";
	
	WebSpec.cameraZoomFactor = 1.5;
	WebSpec.cameraXOffset = 0;
	WebSpec.cameraYOffset = 0;
	
	$("#webspec").mousemove = null;
	
	//Load class images (based on internal class indexes, not selection screen)
	WebSpec.classImages[1].src = "img/scout.png";
	WebSpec.classImages[2].src = "img/sniper.png";
	WebSpec.classImages[3].src = "img/soldier.png";
	WebSpec.classImages[4].src = "img/demoman.png";
	WebSpec.classImages[5].src = "img/medic.png";
	WebSpec.classImages[6].src = "img/heavy.png";
	WebSpec.classImages[7].src = "img/pyro.png"; //Currently heavy icon duplicate
	WebSpec.classImages[8].src = "img/spy.png";
	WebSpec.classImages[9].src = "img/engineer.png";
	
	//and the rest some other time
	
	//CP_* map specifics
	WebSpec.cp_points = new Array();
	
	//KOTH_*
	WebSpec.koth_bluTimer = -1;
	WebSpec.koth_redTimer = -1;
	WebSpec.koth_point = null;
	
	//TODO: PL, CTF, ???
	
	//Canvas
	WebSpec.canvas = document.createElement("canvas");
	WebSpec.screenCanvas = document.createElement("canvas");
	if(!WebSpec.canvas.getContext)
    {
        $("#loading").toggleClass("boxHide");
		$("#errorInitCanvas").toggleClass("boxShow");
        return;
    }
	
	//Sockets
	// FF needs the Moz prefix..
	var host = WebSpec.host;
	
    if(!window.WebSocket)
    {
        if(window.MozWebSocket)
            WebSpec.socket = new MozWebSocket(host, "webspec");
        else
        {
            $("#loading").toggleClass("boxHide");
		    $("#errorInitSockets").toggleClass("boxShow");
            return;
        }
    }
    else
        WebSpec.socket = new WebSocket(host, "webspec");
	
	window.addEventListener('resize',function(){
		WebSpec.width = window.innerWidth;
		WebSpec.height = window.innerHeight;
		WebSpec.canvas.setAttribute('width',WebSpec.width);  
		WebSpec.canvas.setAttribute('height',WebSpec.height);
		WebSpec.screenCanvas.setAttribute('width',WebSpec.width);  
		WebSpec.screenCanvas.setAttribute('height',WebSpec.height);
		
		if (!dbg_resizeKeepZoom)
			calculateZoomFactor();
		calculateCameraOffsets();
	},false);
	
	WebSpec.canvas.setAttribute('width',WebSpec.width);  
    WebSpec.canvas.setAttribute('height',WebSpec.height);
	WebSpec.screenCanvas.setAttribute('width',WebSpec.width);  
    WebSpec.screenCanvas.setAttribute('height',WebSpec.height);
	WebSpec.screenCanvas.style.display = "block";
	$("#webspec").append(WebSpec.screenCanvas);
    $("#webspec").mousemove(function(ev){mousemove(ev);});
    $("#webspec").click(function(ev){mouseclick(ev);});
	WebSpec.screenCTX = WebSpec.screenCanvas.getContext('2d');
	WebSpec.ctx = WebSpec.canvas.getContext('2d'); 
	
	WebSpec.drawTimer = setInterval("draw()", 50);
	
	try {
		WebSpec.socket.onopen = function(msg)
        {
            debug("Connection established");
        };
        
        WebSpec.socket.onmessage = function(msg)
        {
			//TODO change to raw bytes
            var frame = {};
            var offset = 0;
            frame.type = msg.data.charAt(offset);
            offset++;
			
			//if (/*frame.type != 'Z'*/ false) 
				debug("Packet: "+ msg.data);
			
            switch(frame.type)
            {
                // Init
                case "I":
                {
                    var info = 0;
                    frame.map = "";
                    frame.hostname = "";
					frame.team1 = "";
					frame.team2 = "";
                    for (;offset<msg.data.length;offset++)
                    {
                        if (msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
                        if (info == 0)
                            frame.map += msg.data.charAt(offset);
                        else if (info == 1)
                            frame.hostname += msg.data.charAt(offset);
						else if (info == 2)
                            frame.team1 += msg.data.charAt(offset);
						else
                            frame.team2 += msg.data.charAt(offset);
                    }
					
					WebSpec.map = frame.map;
					WebSpec.serverName = frame.hostname;
					WebSpec.team[2] = frame.team1;
					WebSpec.team[3] = frame.team2;
					WebSpec._connected = true;
					
					loadMap();
					
                    break;
                }
                
                // Player connected.
                case "C":
                {
                    var info = 0;
                    frame.userid = "";
					frame.team = "";
					frame.tfclass = "";
					frame.health = "";
					frame.maxHealth = "";
					frame.alive = "";
					frame.ubered = "";
					frame.charge = "";
                    frame.name = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        if(info < 8 && msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
                        if(info == 0)
                            frame.userid += msg.data.charAt(offset);
						else if (info == 1)
							frame.team += msg.data.charAt(offset);
						else if (info == 2)
							frame.tfclass += msg.data.charAt(offset);
						else if (info == 3)
							frame.health += msg.data.charAt(offset);
						else if (info == 4)
							frame.maxHealth += msg.data.charAt(offset);
						else if (info == 5)
							frame.alive += msg.data.charAt(offset);
						else if (info == 6)
							frame.ubered += msg.data.charAt(offset);
						else if (info == 7)
							frame.charge += msg.data.charAt(offset);
                        else
						    frame.name += msg.data.charAt(offset);
                    }
                    
					frame.team = parseInt(frame.team);
					debug(frame.tfclass);
					if (frame.team < 2) //Spectator or unassigned, both treated the same
						WebSpec.teamPlayersAmount[0]++;
					else
						WebSpec.teamPlayersAmount[frame.team-1]++;
					
                    var idx = WebSpec.players.length;
                    WebSpec.players[idx] = {'userid': parseInt(frame.userid), 'name': frame.name, 'team': frame.team, 'tfclass': parseInt(frame.tfclass), 'health': parseInt(frame.health), 'maxHealth': parseInt(frame.maxHealth), 'alive': (parseInt(frame.alive) == 1), 'ubered': (parseInt(frame.ubered) == 1), 'charge': parseInt(frame.charge), 'positions': new Array()};
                    
					WebSpec.chat.push(frame.name +" has joined");
					//DEBUG
					console.log("[chat] "+ frame.name +" has joined");
                    break;
                }
				
				// Player disconnected
                case "D":
                {
                    frame.userid = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        frame.userid += msg.data.charAt(offset);
                    }
                    frame.userid = parseInt(frame.userid);
                    
                    for(var i=0;i<WebSpec.players.length;i++)
                    {
                        if(WebSpec.players[i].userid == frame.userid)
                        {
							var team = WebSpec.players[i].team;
							if (team < 2) //Spectator or unassigned, both treated the same
								WebSpec.teamPlayersAmount[0]--;
							else
								WebSpec.teamPlayersAmount[team-1]--;
							
							$("#chatoutput").append(document.createTextNode(WebSpec.players[i].name+" has left the server :'("));
							$("#chatoutput").append("\n");
							$('#chatoutput').prop('scrollTop', $('#chatoutput').prop('scrollHeight'));
                            
                            WebSpec.players.splice(i, 1);
                            break;
                        }
                    }
                    break;
                }
                
                // Game message
                case "G":
                {
                    var info = 0;
                    frame.userid = "";
                    frame.msg = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        if(info == 0 && msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
                        if(info == 0)
                            frame.userid += msg.data.charAt(offset);
                        else
                            frame.msg += msg.data.charAt(offset);
                    }
                    frame.userid = parseInt(frame.userid);
                    
					var idx = -2;
					var playerName = "";
                    // Worldspawn/"Console"
                    if(frame.userid == 0)
                    {
                        playerName = "Console";
						idx = -1;
                    }
                    
                    for(var i=0;i<WebSpec.players.length;i++)
                    {
                        if(WebSpec.players[i].userid == frame.userid)
                        {
                            idx = i;
							playerName = WebSpec.players[i].name;
                            break;
                        }
                    }
                    if(idx != -2)
                    {
                        $("#chatoutput").append(document.createTextNode(playerName +": "+ frame.msg));
						$("#chatoutput").append("\n");
						$('#chatoutput').prop('scrollTop', $('#chatoutput').prop('scrollHeight'));
                    }
                    break;
                }
				
				// Someone killed somebody
                case "K":
                {
                    if(WebSpec.ctx == null)
                        break;
                    
                    var info = 0;
                    frame.victim = "";
                    frame.attacker = "";
                    frame.weapon = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        if(info < 2 && msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
                        if(info == 0)
                            frame.victim += msg.data.charAt(offset);
                        else if(info == 1)
                            frame.attacker += msg.data.charAt(offset);
                        else
                            frame.weapon += msg.data.charAt(offset);
                    }
                    
                    frame.attacker = parseInt(frame.attacker);
                    frame.victim = parseInt(frame.victim);
                    
                    var idxV = -1;
                    var idxA = -1;
                    for(var i=0;i<WebSpec.players.length;i++)
                    {
                        if(WebSpec.players[i].userid == frame.attacker)
                        {
                            idxA = i;
                        }
                        if(WebSpec.players[i].userid == frame.victim)
                        {
                            idxV = i;
                        }
                    }
                    
                    var attackername = "";
                    var attackerteam = 0;
                    if(frame.attacker == 0)
                        attackername = "WORLD";
                    else if(idxA == -1)
                        attackername = "NotFound(#"+frame.attacker+")";
                    else
                    {
                        //WebSpec.players[idxA].frags++;
                        attackername = WebSpec.players[idxA].name;
                        attackerteam = WebSpec.players[idxA].team;
                    }
                    
                    var victimname = "";
                    var victimteam = 0;
                    if(idxV == -1)
                        victimname = "NotFound(#"+frame.victim+")";
                    else
                    {
                        victimname = WebSpec.players[idxV].name;
                        victimteam = WebSpec.players[idxV].team;
                        WebSpec.players[idxV].alive = false;
                        WebSpec.players[idxV].health = 0;
                        //WebSpec.players[idxV].deaths++;
                        //WebSpec.teamPlayersAlive[WebSpec.players[idxV].team-2]--;
                        if(WebSpec.players[idxV].positions.length != 0)
                        {
                            WebSpec.players[idxV].positions[WebSpec.players[idxV].positions.length-1].diedhere = true;
                        }
                    }
                    
                    // Suicides = frags-1
                    if(idxV != -1 && idxA != -1 && idxV == idxA)
                    {
                        // We added one frag already above..
                        //WebSpec.players[idxV].frags-=2;
                    }
                    
                    var d = new Date();
                    //SourceTV2D.frags[SourceTV2D.frags.length] = {'attacker': attackername, 'ateam': attackerteam, 'victim': victimname, 'vteam': victimteam, 'weapon': frame.weapon, 'time': (d.getTime()/1000)};
                    
                    //sortScoreBoard();
                    
                    //debug("Player "+attackername+" killed "+victimname+" with "+frame.weapon);
                    break;
                }
				
				// Player changed his name
                case "N":
                {
                    var info = 0;
                    frame.userid = "";
                    frame.name = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        if(msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
                        if(info == 0)
                            frame.userid += msg.data.charAt(offset);
                        else
                            frame.name += msg.data.charAt(offset);
                    }
                    frame.userid = parseInt(frame.userid);
                    
                    var idx = -1;
                    for(var i=0;i<WebSpec.players.length;i++)
                    {
                        if(WebSpec.players[i].userid == frame.userid)
                        {
                            idx = i;
                            break;
                        }
                    }
                    if(idx != -1)
                    {
                        var d = new Date();
                        WebSpec.players[idx].name = frame.name;
                    }
                    break;
                }
				
                // Player spawned
                case "S":
                {
					var info = 0
                    frame.userid = "";
					frame.tfclass = "";
					frame.health = "";
					frame.maxHealth = "";
					
					debug("spawn: "+ msg.data);
					
                    for(;offset<msg.data.length;offset++)
                    {
                        if(info < 3 && msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
						
                        if(info == 0)
                            frame.userid += msg.data.charAt(offset);
						else if (info == 1)
							frame.tfclass += msg.data.charAt(offset);
						else if (info == 2)
							frame.health += msg.data.charAt(offset);
                        else
                            frame.maxHealth += msg.data.charAt(offset);
                    }
                    frame.userid = parseInt(frame.userid);
					frame.tfclass = parseInt(frame.tfclass)
                    var idx = -1;
                    for(var i=0;i<WebSpec.players.length;i++)
                    {
                        if(WebSpec.players[i].userid == frame.userid)
                        {
                            idx = i;
                            break;
                        }
                    }
                    if(idx != -1)
                    {
                        //if(!WebSpec.players[idx].alive)
                            //WebSpec.teamPlayersAlive[WebSpec.players[idx].team-2]++;
                        WebSpec.players[idx].alive = true;
						if (frame.tfclass > 0)
							WebSpec.players[idx].tfclass = parseInt(frame.tfclass);
                        WebSpec.players[idx].health = parseInt(frame.health);
                        WebSpec.players[idx].maxHealth = parseInt(frame.maxHealth);
                    }
                    break;
                }
				
				// Player hurt/healed
				case "H":
				{
					var info = 0;
					frame.hType = parseInt(msg.data.charAt(offset)); //0 = hurt, 1 = heal
                    frame.userid = "";
                    frame.amount = "";
					offset++;
                    for(;offset<msg.data.length;offset++)
                    {
                        if(msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
						if (info == 0)
							frame.userid += msg.data.charAt(offset);
                        else
                            frame.amount += msg.data.charAt(offset);
                    }
                    frame.userid = parseInt(frame.userid);
                    frame.amount = parseInt(frame.amount);
					
					if (frame.hType == 1)
						frame.amount = -frame.amount;
                    
                    var idx = -1;
                    for(var i=0;i<WebSpec.players.length;i++)
                    {
                        if(WebSpec.players[i].userid == frame.userid)
                        {
                            idx = i;
                            break;
                        }
                    }
                    if(idx != -1)
                    {
                        WebSpec.players[idx].health =  WebSpec.players[idx].health - frame.amount;
                        if(WebSpec.players[idx].health < 0)
                            WebSpec.players[idx].health = 0;
                        /*if(WebSpec.players[idx].positions.length > 0)
                            WebSpec.players[idx].positions[WebSpec.players[idx].positions.length-1].hurt = true;*/
                    }
                    break;
                }
				
				// Player positions updated
                case "O":
                {
                    if(!WebSpec.mapSettingsLoaded || WebSpec.background == null)
                        break;
                    
                    var info = 0;
                    var playerIndex = 0;
                    frame.positions = new Array();
					frame.health = "";
					frame.charge = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        // next player
                        if(msg.data.charAt(offset) == '|')
                        {
                            info = 0;
                            playerIndex++;
                            continue;
                        }
                        
                        if(msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
						
                        if(frame.positions[playerIndex] == undefined)
                            frame.positions[playerIndex] = new Array('','','','','', '');
                        
                        frame.positions[playerIndex][info] += msg.data.charAt(offset);
                    }
                    
                    // Save the player positions
                    var idx = -1;
                    var d = new Date();
                    var time = d.getTime();
                    for(var i=0;i<frame.positions.length;i++)
                    {
                        frame.positions[i][0] = parseInt(frame.positions[i][0]);
                        frame.positions[i][1] = parseInt(frame.positions[i][1]);
                        frame.positions[i][2] = parseInt(frame.positions[i][2]);
                        frame.positions[i][3] = parseInt(frame.positions[i][3]);
                        
                        frame.positions[i][3] += 90;
                        
                        if(frame.positions[i][3] < 0)
                            frame.positions[i][3] *= -1;
                        else if(frame.positions[i][3] > 0)
                            frame.positions[i][3] = 360-frame.positions[i][3];
                        
                        frame.positions[i][3] = (Math.PI/180)*frame.positions[i][3];
                        
                        //frame.positions[i][4] = parseInt(frame.positions[i][4]);
                        if(WebSpec.mapSettings.flipx) {
                            frame.positions[i][1] *= -1;
							console.log("flip");
						}
                        if(WebSpec.mapSettings.flipy)
                            frame.positions[i][2] *= -1;
                        
                        frame.positions[i][1] = Math.round(((frame.positions[i][1] + WebSpec.mapSettings.xoffset) / WebSpec.mapSettings.scale) * WebSpec.scaling);
                        frame.positions[i][2] = Math.round(((frame.positions[i][2] + WebSpec.mapSettings.yoffset) / WebSpec.mapSettings.scale) * WebSpec.scaling);
                        
                        // Get the correct team color
                        idx = -1;
                        for(var p=0;p<WebSpec.players.length;p++)
                        {
                            if(WebSpec.players[p].userid == frame.positions[i][0])
                            {
                                idx = p;
                                break;
                            }
                        }
                        
                        if(idx != -1)
                        {
                            WebSpec.players[idx].positions[WebSpec.players[idx].positions.length] = {'x': frame.positions[i][1], 'y': frame.positions[i][2], 'angle': frame.positions[i][3], 'time': time, 'diffx': null, 'diffy': null, 'swapx': null, 'swapy': null, 'diedhere': false, 'hurt': false};
							WebSpec.players[idx].health = parseInt(frame.positions[i][4]);
							WebSpec.players[idx].charge = parseInt(frame.positions[i][5]);
                        }
                        
                        //debug("Player moved: #"+frame.positions[i][0]+" - x: "+frame.positions[i][1]+", y: "+frame.positions[i][2]+", angle: "+frame.positions[i][3]);
                    }
                    
                    break;
                }
				
				//Team info changed
				case "B":
				{
					var info = 0;
					frame.teamid = ""; //change to int
					frame.teamname = "";
					frame.readystate = ""; //cange to int
					
					for(;offset<msg.data.length;offset++)
					{
						if(msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
						
						if (info == 0)
							frame.teamid += msg.data.charAt(offset);
						else if (info == 1)
							frame.teamname += msg.data.charAt(offset);
						else 
							frame.readystate += msg.data.charAt(offset);
					}
					
					frame.teamid = parseInt(frame.teamid);
					frame.readystate = parseInt(frame.readystate);
					
					WebSpec.team[frame.teamid] = frame.teamname;
					WebSpec.teamReadyState[frame.teamid] = frame.readystate;
					
					debug("Team \""+ frame.teamname +"\" has readystate "+ frame.readystate);
				}
				
				// Player changed team
                case "T":
                {
                    var info = 0;
                    frame.userid = "";
                    frame.team = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        if(msg.data.charAt(offset) == ':')
                        {
                            info++;
                            continue;
                        }
                        if(info == 0)
                            frame.userid += msg.data.charAt(offset);
                        else
                            frame.team += msg.data.charAt(offset);
                    }
                    frame.userid = parseInt(frame.userid);
                    frame.team = parseInt(frame.team);
                    
                    var idx = -1;
                    for(var i=0;i<WebSpec.players.length;i++)
                    {
                        if(WebSpec.players[i].userid == frame.userid)
                        {
                            idx = i;
                            break;
                        }
                    }
                    if(idx != -1)
                    {
                        // He joined that team
                        if(frame.team < 2)
                            WebSpec.teamPlayersAmount[0]++;
                        else
                            WebSpec.teamPlayersAmount[frame.team-1]++;
						
                        // He left that team
                        if(WebSpec.players[idx].team < 2)
                            WebSpec.teamPlayersAmount[0]--;
                        else
                            WebSpec.teamPlayersAmount[SourceTV2D.players[idx].team-1]--;
                        
                        var d = new Date();
                        WebSpec.players[idx].team = frame.team;
                        
                        if(WebSpec.players[idx].team < 2)
                            WebSpec.players[idx].positions.clear();
                    }
                    else
                        debug("NOT FOUND!!! Player #"+frame.userid+" changed team to: "+frame.team);
                    break;
                }
                
                // Spectator amount changed
                case "A":
                {
                    frame.totalwatching = "";
                    for(;offset<msg.data.length;offset++)
                    {
                        frame.totalwatching += msg.data.charAt(offset);
                    }
                    frame.totalwatching = parseInt(frame.totalwatching);
                    WebSpec.totalUsersWatching = frame.totalwatching;
                    
                    break;
				}
            }
        };
		
        WebSpec.socket.onerror = function(msg)
        {
            debug("Socket reported error!");
        };
        WebSpec.socket.onclose = function(msg)
        {
			WebSpec._disconnectRS = this.readyState;
			WebSpec._disconnectCode = msg.code;
			WebSpec._disconnectReason = msg.reason;
			WebSpec._disconnectClean = msg.clean;
			
			WebSpec._connected = true;
			WebSpec._disconnected = true;
            debug("Disconnected - readyState: "+this.readyState+" Code: "+msg.code+". Reason:"+msg.reason+" - wasClean: "+msg.wasClean);
        };
	} catch(ex){
        debug('Error: '+ex);
    }
}

function draw() {
	try {
		//Clear
		WebSpec.screenCTX.clearRect(0,0,WebSpec.width,WebSpec.height);
		WebSpec.ctx.clearRect(0,0,WebSpec.width,WebSpec.height);
		
		//Not connected just draw message and return
		if (!WebSpec._connected) {
			WebSpec.screenCTX.save();
            WebSpec.screenCTX.beginPath();
            WebSpec.screenCTX.fillStyle = "rgb(0, 0, 0)";
            WebSpec.screenCTX.rect(0, 0, WebSpec.width, WebSpec.height);
            WebSpec.screenCTX.fill();
			
			WebSpec.screenCTX.textAlign = "center";
			WebSpec.screenCTX.font = "24pt Roboto Condensed";
			WebSpec.screenCTX.fillStyle = "rgb(255, 255, 255)";
			WebSpec.screenCTX.fillText("Connecting...", WebSpec.width/2, WebSpec.height/2 - 20);
			WebSpec.screenCTX.fillStyle = "rgba(255, 255, 255, 0.8)";
			WebSpec.screenCTX.fillText("Server may be down, or testing new version", WebSpec.width/2, WebSpec.height/2 + 10);
            WebSpec.screenCTX.restore();
			return;
		} else if (WebSpec._disconnected) { //If we became disconnected, show error/reason
			WebSpec.screenCTX.fillStyle = "rgb(0, 0, 0)";
			WebSpec.screenCTX.fillRect(0, 0, WebSpec.width, WebSpec.height);
			
			WebSpec.screenCTX.textAlign = "center";
			WebSpec.screenCTX.font = "24pt Roboto Condensed";
			WebSpec.screenCTX.fillStyle = "rgb(255, 255, 255)";
			WebSpec.screenCTX.fillText("Disconnected!", WebSpec.width/2, WebSpec.height/2 - 20);
			
			WebSpec.screenCTX.font = "14pt Roboto Condensed";
			WebSpec.screenCTX.fillStyle = "rgba(255, 255, 255, 0.8)";
			WebSpec.screenCTX.fillText("- Details -", WebSpec.width/2, WebSpec.height/2 + 20);
			WebSpec.screenCTX.fillText(WebSpec._disconnectRS +" - "+ WebSpec._disconnectCode +" - "+ WebSpec._disconnectClean, WebSpec.width/2, WebSpec.height/2 + 40);
			WebSpec.screenCTX.fillText(WebSpec._disconnectReason, WebSpec.width/2, WebSpec.height/2 + 60);
			return;
		}
		
		// 
		// Map & players
		// 
		
		//Shorthand variables
		var cameraX = WebSpec.cameraXOffset;
		var cameraY = WebSpec.cameraYOffset;
		var cameraZoom = WebSpec.cameraZoomFactor;
		var shouldScaleDown = (WebSpec.width < 1024);
		var scaleDown = (shouldScaleDown) ? (WebSpec.width/1024) : 1.0;
		
		//Draw background (or black if not loaded yet)
		if (WebSpec.background != null) 
			WebSpec.ctx.drawImage(WebSpec.background, cameraX, cameraY, WebSpec.background.width*cameraZoom, WebSpec.background.height*cameraZoom);
			//WebSpec.ctx.drawImage(WebSpec.background, WebSpec.width/2 - WebSpec.background.width/2, WebSpec.height/2 - WebSpec.background.height/2,WebSpec.background.width,WebSpec.background.height);
        else
        {
            WebSpec.ctx.save();
            WebSpec.ctx.beginPath();
            WebSpec.ctx.fillStyle = "rgb(0, 0, 0)";
            WebSpec.ctx.rect(0, 0, WebSpec.width, WebSpec.height);
            WebSpec.ctx.fill();
            WebSpec.ctx.restore();
        }
		
		var d = new Date();
        var time = d.getTime()/1000;
		
		//Draw players
		WebSpec.ctx.font = Math.round(8*scaleDown) +"pt Verdana";
		for(var i=0;i<WebSpec.players.length;i++)
        {
            // Make sure we're in sync with the other messages..
            // Delete older frames
            while(WebSpec.players[i].positions.length > 0 && (time - WebSpec.players[i].positions[0].time) > 500) //originally 2000
            {
              WebSpec.players[i].positions.splice(0,1);
            }
			
            // There is no coordinate for this player yet
            if(WebSpec.players[i].positions.length == 0)
                continue;
            
            WebSpec.ctx.save();
			
            if(WebSpec.players[i].team < 2)
                WebSpec.ctx.fillStyle = "black";
            else if(WebSpec.players[i].team == 2)
            {
                if(WebSpec.players[i].positions[0].diedhere == false)
                    WebSpec.ctx.fillStyle = "red";
                else
                    WebSpec.ctx.fillStyle = "rgba(255,0,0,0.3)";
            }
            else if(WebSpec.players[i].team == 3)
            {
                if(WebSpec.players[i].positions[0].diedhere == false)
                    WebSpec.ctx.fillStyle = "blue";
                else
                    WebSpec.ctx.fillStyle = "rgba(0,0,255,0.3)";
            }
            
            // Teleport directly to new spawn, if he died at this position
            if(WebSpec.players[i].positions[0].diedhere)
            {
                if(WebSpec.players[i].positions[1])
                {
                    //if(time >= WebSpec.players[i].positions[1].time)
                        WebSpec.players[i].positions.splice(0,1);
                }
            }
            // Move the player smoothly towards the new position
            else if(WebSpec.players[i].positions.length > 1)
            {
                if(WebSpec.players[i].positions[0].x == WebSpec.players[i].positions[1].x
                && WebSpec.players[i].positions[0].y == WebSpec.players[i].positions[1].y)
                {
                    //if(time >= WebSpec.players[i].positions[1].time)
                        WebSpec.players[i].positions.splice(0,1);
                }
                else
                {
                    // This function is called 20x a second
                    if(WebSpec.players[i].positions[0].swapx == null)
                    {
                        WebSpec.players[i].positions[0].swapx = WebSpec.players[i].positions[0].x > WebSpec.players[i].positions[1].x?-1:1;
                        WebSpec.players[i].positions[0].swapy = WebSpec.players[i].positions[0].y > WebSpec.players[i].positions[1].y?-1:1;
                    }
                    if(WebSpec.players[i].positions[0].diffx == null)
                    {
                        var timediff = WebSpec.players[i].positions[1].time - WebSpec.players[i].positions[0].time;
                        WebSpec.players[i].positions[0].diffx = Math.abs(WebSpec.players[i].positions[1].x - WebSpec.players[i].positions[0].x)/(timediff/50);
                        WebSpec.players[i].positions[0].diffy = Math.abs(WebSpec.players[i].positions[1].y - WebSpec.players[i].positions[0].y)/(timediff/50);
                    }
                    
                    var x = WebSpec.players[i].positions[0].x + WebSpec.players[i].positions[0].swapx*WebSpec.players[i].positions[0].diffx;
                    var y = WebSpec.players[i].positions[0].y + WebSpec.players[i].positions[0].swapy*WebSpec.players[i].positions[0].diffy;
                    
                    // We're moving too far...
                    if((WebSpec.players[i].positions[0].swapx==-1 && x <= WebSpec.players[i].positions[1].x)
                    || (WebSpec.players[i].positions[0].swapx==1 && x >= WebSpec.players[i].positions[1].x)
                    || (WebSpec.players[i].positions[0].swapy==-1 && y <= WebSpec.players[i].positions[1].y)
                    || (WebSpec.players[i].positions[0].swapy==1 && y >= WebSpec.players[i].positions[1].y))
                    {
                        WebSpec.players[i].positions.splice(0,1);
                    }
                    else
                    {
                        WebSpec.players[i].positions[0].x = x;
                        WebSpec.players[i].positions[0].y = y;
                    }
                }
            }
            
            var playerRadius = WebSpec.playerRadius;
            // User hovers his mouse over this player
            /*if(WebSpec.players[i].hovered || WebSpec.players[i].selected)
            {
                playerRadius = WebSpec.playerRadius + 4*WebSpec.scaling;
                WebSpec.ctx.save();
                WebSpec.ctx.beginPath();
                WebSpec.ctx.fillStyle = "rgba(255, 255, 255, 0.8)";
                WebSpec.ctx.arc((WebSpec.players[i].positions[0].x + cameraX)*cameraZoom, (WebSpec.players[i].positions[0].y + cameraY)*cameraZoom, playerRadius+2*WebSpec.scaling*cameraZoom, 0, Math.PI*2, true);
                WebSpec.ctx.fill();
                WebSpec.ctx.restore();
            }*/

            // Draw player itself
            WebSpec.ctx.beginPath();
            WebSpec.ctx.arc(((WebSpec.players[i].positions[0].x) * cameraZoom) + cameraX, ((WebSpec.players[i].positions[0].y) * cameraZoom)+cameraY, playerRadius*cameraZoom, 0, Math.PI*2, true);
            WebSpec.ctx.fill();
			
			// Draw name TODO account for rotation
			WebSpec.ctx.translate(((WebSpec.players[i].positions[0].x) * cameraZoom)+cameraX, ((WebSpec.players[i].positions[0].y) * cameraZoom) + cameraY);
			WebSpec.ctx.rotate(dtor(WebSpec.mapSettings.rotation));
			WebSpec.ctx.textAlign = "center";
			WebSpec.ctx.strokeStyle = WebSpec.ctx.fillStyle;
			WebSpec.ctx.fillStyle = "rgba(255, 255, 255, 0.7)";
			WebSpec.ctx.lineWidth = 3*scaleDown;
			WebSpec.ctx.strokeText(WebSpec.players[i].name, 0, -5 * cameraZoom);
			WebSpec.ctx.fillText(WebSpec.players[i].name, 0, -5 * cameraZoom);
			WebSpec.ctx.rotate(-dtor(WebSpec.mapSettings.rotation));
			WebSpec.ctx.translate(-(((WebSpec.players[i].positions[0].x) * cameraZoom)+cameraX), -(((WebSpec.players[i].positions[0].y) * cameraZoom) + cameraY));
            
			
            // Draw view angle as white dot
            WebSpec.ctx.translate(((WebSpec.players[i].positions[0].x) *cameraZoom) + cameraX, ((WebSpec.players[i].positions[0].y) * cameraZoom) + cameraY);
            WebSpec.ctx.fillStyle = "white";
            WebSpec.ctx.rotate(WebSpec.players[i].positions[0].angle);
            WebSpec.ctx.beginPath();
            WebSpec.ctx.arc(0, Math.round(1.5 * WebSpec.scaling*cameraZoom), Math.round(1 * WebSpec.scaling*cameraZoom), 0, Math.PI*2, true);
            WebSpec.ctx.fill();
            
            WebSpec.ctx.restore();
        }
		
		//
		// HUD
		//
		
		WebSpec.screenCTX.translate(WebSpec.width/2, WebSpec.height/2);
		WebSpec.screenCTX.rotate(dtor(WebSpec.mapSettings.rotation));
		WebSpec.screenCTX.drawImage(WebSpec.canvas, -WebSpec.width / 2, -WebSpec.height / 2, WebSpec.width, WebSpec.height);
		WebSpec.screenCTX.rotate(-dtor(WebSpec.mapSettings.rotation));
		WebSpec.screenCTX.translate(-WebSpec.width/2, -WebSpec.height/2);
		
		//Team titles & scores (width variable, min 1024px; height 50px, sub height 30px) (scale down is screen not wide enough)
		//Sub 1024 scale everything down, dont bother with gradient
		var scoreBGWidth = WebSpec.width;
		var scoreBGHeight = 40;
		var scoreBGFill = null;
		
		var teamScoreFontSize = 16;
		var teamScoreFontPos = new Array(12, 26);
		var teamScoreNameFontSize = 14;
		var teamScoreNameFontPos = new Array(36, 26);
		var teamScoreSeparator = 8;
		WebSpec.screenCTX.font = Math.round(teamScoreNameFontSize) +"pt Roboto Condensed";
		var teamScoreSize = new Array(Math.max(Math.max(WebSpec.screenCTX.measureText(WebSpec.team[2]).width,WebSpec.screenCTX.measureText(WebSpec.team[3]).width) + 60, 135), 30); //135px min width
		var teamScorePos = new Array(WebSpec.width/2 - teamScoreSize[0], 5);
		
		var playerListBoxWidth = 210;
		var playerListBoxHeightB = 4 + (24*WebSpec.teamPlayersAmount[2]);
		var playerListBoxHeightR = 4 + (24*WebSpec.teamPlayersAmount[1]);
		var playerListBoxPosB = 0;
		var playerListBoxPosR = 0;
		var playerListIconSize = new Array(22, 20);
		var playerListHealthSize = new Array(36, 20);
		var playerListNameSize = new Array(136, 20);
		var playerListFontSize = 12;
		var playerListYSeparator = 4;
		var playerListPlayerSeparator = 24;
		var playerListClassImageX = 4;
		var playerListHealthX = 30;
		var playerListNameX = 70;
		
		if (shouldScaleDown) {
			scoreBGHeight *= scaleDown;
			scoreBGFill = "rgba(0, 0, 0, 0.85)";
			
			teamScoreSize[0] *= scaleDown;
			teamScoreSize[1] *= scaleDown;
			teamScorePos[0] = WebSpec.width/2 - teamScoreSize[0];
			teamScorePos[1] *= scaleDown;
			teamScoreFontSize *= scaleDown;
			teamScoreNameFontSize *= scaleDown;
			teamScoreFontPos[0] *= scaleDown;
			teamScoreFontPos[1] *= scaleDown;
			teamScoreNameFontPos[0] *= scaleDown;
			teamScoreNameFontPos[1] *= scaleDown;
			teamScoreSeparator *= scaleDown;
			
			playerListBoxWidth *= scaleDown;
			playerListBoxHeightB *= scaleDown;
			playerListBoxHeightR *= scaleDown;
			playerListIconSize[0] *= scaleDown;
			playerListIconSize[1] *= scaleDown;
			playerListHealthSize[0] *= scaleDown;
			playerListHealthSize[1] *= scaleDown;
			playerListNameSize[0] *= scaleDown;
			playerListNameSize[1] *= scaleDown;
			playerListFontSize *= scaleDown;
			playerListYSeparator *= scaleDown;
			playerListPlayerSeparator *= scaleDown;
			playerListClassImageX *= scaleDown;
			playerListHealthX *= scaleDown;
			playerListNameX *= scaleDown;
		} else {
			scoreBGFill = WebSpec.screenCTX.createLinearGradient(0, 0, scoreBGWidth, scoreBGHeight);
			scoreBGFill.addColorStop(0, "rgba(0, 0, 0, 0)");
			scoreBGFill.addColorStop(0.25, "rgba(0, 0, 0, 0.85)");
			scoreBGFill.addColorStop(0.75, "rgba(0, 0, 0, 0.85)");
			scoreBGFill.addColorStop(1, "rgba(0, 0, 0, 0)");
		}
		
		WebSpec.screenCTX.fillStyle = scoreBGFill;
		WebSpec.screenCTX.fillRect(0, 0, scoreBGWidth, scoreBGHeight);
		
		// Team BGs
		WebSpec.screenCTX.fillStyle = "#416376";
		WebSpec.screenCTX.fillRect(teamScorePos[0], teamScorePos[1], teamScoreSize[0], teamScoreSize[1]);
		WebSpec.screenCTX.fillStyle = "#234558";
		WebSpec.screenCTX.fillRect(teamScorePos[0]+teamScoreSize[0] - teamScoreSize[1], teamScorePos[1], teamScoreSize[1], teamScoreSize[1]);
		WebSpec.screenCTX.fillStyle = "#a24033";
		WebSpec.screenCTX.fillRect(teamScorePos[0] + teamScoreSize[0] + teamScoreSeparator, teamScorePos[1], teamScoreSize[0], teamScoreSize[1]);
		WebSpec.screenCTX.fillStyle = "#842215";
		WebSpec.screenCTX.fillRect(teamScorePos[0] + teamScoreSize[0] + teamScoreSeparator, teamScorePos[1], teamScoreSize[1], teamScoreSize[1]);
		
		// Team names
		WebSpec.screenCTX.textAlign = "right";
		WebSpec.screenCTX.fillStyle = "white";
		WebSpec.screenCTX.font = Math.round(teamScoreNameFontSize) +"pt Roboto Condensed";
		WebSpec.screenCTX.fillText(WebSpec.team[3], teamScorePos[0] + teamScoreSize[0] - teamScoreNameFontPos[0], teamScoreNameFontPos[1]);
		WebSpec.screenCTX.textAlign = "left";
		WebSpec.screenCTX.fillText(WebSpec.team[2], teamScorePos[0] + teamScoreSize[0] + teamScoreSeparator + teamScoreNameFontPos[0], teamScoreNameFontPos[1]);
		
		// Team scores
		WebSpec.screenCTX.textAlign = "center";
		//WebSpec.screenCTX.font = Math.round(teamScoreFontSize) +"pt Roboto Condensed";
		WebSpec.screenCTX.fillText(WebSpec.teamPoints[1], teamScorePos[0] + teamScoreSize[0] - (teamScoreSize[1]/2), teamScoreFontPos[1]);
		WebSpec.screenCTX.fillText(WebSpec.teamPoints[0], teamScorePos[0] + teamScoreSize[0] + teamScoreSeparator + (teamScoreSize[1]/2), teamScoreFontPos[1]);
		
		//Draw player lists
		//BGs
		var playerListBoxYPosB = WebSpec.height/2 - playerListBoxHeightB/2;
		var playerListBoxYPosR = WebSpec.height/2 - playerListBoxHeightR/2;
		
		WebSpec.screenCTX.fillStyle = "rgba(24, 24, 24, 0.85)";
		if (WebSpec.teamPlayersAmount[2] > 0)
			WebSpec.screenCTX.fillRect(0, playerListBoxYPosB, playerListBoxWidth, playerListBoxHeightB); //Blu
		if (WebSpec.teamPlayersAmount[1] > 0)
			WebSpec.screenCTX.fillRect(WebSpec.width - playerListBoxWidth, playerListBoxYPosR, playerListBoxWidth, playerListBoxHeightR); //Red
		
		//Players
		var playerListIndexB = 0;
		var playerListIndexR = 0;
		WebSpec.screenCTX.font = Math.round(playerListFontSize) +"pt Roboto Condensed";
		WebSpec.screenCTX.textAlign = "center";
		for (var i=0; i<WebSpec.players.length; i++) {
			var player = WebSpec.players[i];
			var teamX = 0;
			var teamY = playerListBoxYPosB;
			var teamFill = "rgba(65, 99, 118, 0.85)";
			var playerY = (playerListPlayerSeparator * playerListIndexB);
			playerListIndexB++;
			if (player.team < 2)
				continue;
			else if (player.team == 2) {
				teamX = WebSpec.width - playerListBoxWidth;
				teamY = playerListBoxYPosR;
				teamFill = "rgba(162, 64, 51, 0.85)";
				playerY = (playerListPlayerSeparator * playerListIndexR);
				playerListIndexB--;
				playerListIndexR++;
			}
			
			//Draw class (switch to player.class when ready) TODO
			var classImage = WebSpec.classImages[player.tfclass];
			if (classImage != null)
				WebSpec.screenCTX.drawImage(classImage, teamX + playerListClassImageX, teamY + playerListYSeparator + playerY, playerListIconSize[0], playerListIconSize[1]);
			
			//Draw health (switch to player.health when ready) TODO
			var drawHealthBG = (player.health > 0);
			var drawHealthFancyColour = (player.health > player.maxHealth || player.health <= player.maxHealth/2);
			var drawHealthActualColour = "white";
			var drawHealthBGActualColour = "rgb(105, 99, 86)";
			if (drawHealthFancyColour) {
				drawHealthActualColour = (player.health > player.maxHealth) ? "rgb(126, 226, 24)" : "rgb(255, 201, 24)";
				drawHealthBGActualColour = (player.health <= player.maxHealth/2) ? "rgb(110, 18, 18)" : drawHealthBGActualColour;
			}
			
			WebSpec.screenCTX.textAlign = "center";
			if (drawHealthBG) {
				WebSpec.screenCTX.fillStyle = drawHealthBGActualColour;
				var healthBGYOffset = (player.health >= player.maxHealth) ? 0 : playerListHealthSize[1] - (player.health/player.maxHealth)*playerListHealthSize[1];
				WebSpec.screenCTX.fillRect(teamX + playerListHealthX, teamY + playerListYSeparator + playerY + healthBGYOffset, playerListHealthSize[0], playerListHealthSize[1] - healthBGYOffset);
			}
			WebSpec.screenCTX.fillStyle = "black";
			WebSpec.screenCTX.fillText(player.health, teamX + playerListHealthX + playerListHealthSize[0]/2 + 1, teamY + playerListYSeparator + playerY + playerListHealthSize[1]*0.75 + 1);
			WebSpec.screenCTX.fillStyle = drawHealthActualColour;
			WebSpec.screenCTX.fillText(player.health, teamX + playerListHealthX + playerListHealthSize[0]/2, teamY + playerListYSeparator + playerY + playerListHealthSize[1]*0.75);
			
			//Draw name
			WebSpec.screenCTX.textAlign = "left";
			WebSpec.screenCTX.fillStyle = drawHealthBG ? teamFill : "rgba(255, 255, 255, 0.25)";
			WebSpec.screenCTX.fillRect(teamX + playerListNameX, teamY + playerListYSeparator + playerY, playerListNameSize[0], playerListNameSize[1]);
			WebSpec.screenCTX.fillStyle = "black";
			var isMedic = (player.tfclass == TFClass.Medic);
			var chargeWidth = (isMedic) ? WebSpec.screenCTX.measureText(player.charge+"%").width + playerListYSeparator : 0;
			var playerName = truncateString(WebSpec.screenCTX, player.name, playerListNameSize[0] - playerListYSeparator*2 - chargeWidth);
			WebSpec.screenCTX.fillText(playerName, teamX + playerListNameX + playerListYSeparator + 1, teamY + playerListYSeparator + playerY + playerListNameSize[1]*0.75 + 1);
			WebSpec.screenCTX.fillStyle = "white";
			WebSpec.screenCTX.fillText(playerName, teamX + playerListNameX + playerListYSeparator, teamY + playerListYSeparator + playerY + playerListNameSize[1]*0.75);
			if (isMedic) {
				WebSpec.screenCTX.textAlign = "right";
				WebSpec.screenCTX.fillStyle = "black";
				WebSpec.screenCTX.fillText(player.charge+"%", teamX + playerListNameX + playerListNameSize[0] - playerListYSeparator + 1, teamY + playerListYSeparator + playerY + playerListNameSize[1]*0.75 + 1);
				WebSpec.screenCTX.fillStyle = "rgb(24, 226, 226)";
				WebSpec.screenCTX.fillText(player.charge+"%", teamX + playerListNameX + playerListNameSize[0] - playerListYSeparator, teamY + playerListYSeparator + playerY + playerListNameSize[1]*0.75);
			}
		}
		
		//Connect notice
		WebSpec.screenCTX.textAlign = "center";
		WebSpec.font = "16pt Verdana";
		WebSpec.screenCTX.lineWidth = 4;
		WebSpec.screenCTX.fillStyle = "rgba(0, 0, 0, 0.75)";
		WebSpec.screenCTX.fillRect(0, WebSpec.height - 50, WebSpec.width, 50);
		WebSpec.screenCTX.fillStyle = "white";
		WebSpec.screenCTX.strokeStyle = "rgba(0, 0, 255, 0.3)";
		WebSpec.screenCTX.strokeText("Live TF2 WebSpectator. Server private atm.", WebSpec.width/2, WebSpec.height - 24);
		WebSpec.screenCTX.fillText("Live TF2 WebSpectator. Server private atm.", WebSpec.width/2, WebSpec.height - 24);
		WebSpec.screenCTX.font = "8pt Verdana";
		WebSpec.screenCTX.fillText("Locked to cp_gullywash_final1 for testing, more maps when stable", WebSpec.width/2, WebSpec.height - 8);
		WebSpec.screenCTX.textAlign = "left";
		WebSpec.screenCTX.font = "8pt Verdana";
		WebSpec.screenCTX.fillText("Inspired by jimbomcb's Web Spectator, based on SourceTV2D", 8, WebSpec.height - 28);
		WebSpec.screenCTX.fillText("HUD based on omp's hud (rawr.am/tf2hud) & TF.TV score overlay", 8, WebSpec.height - 8);
		WebSpec.screenCTX.textAlign = "right";
		WebSpec.screenCTX.font = "8pt Verdana";
		WebSpec.screenCTX.fillText("Current todo: fix class icons, respawn time, heal beam,", WebSpec.width - 8, WebSpec.height - 28);
		WebSpec.screenCTX.fillText("player conditions, UI scaling, cool stuff, move from SourceMod!", WebSpec.width - 8, WebSpec.height - 8);
		
	} catch (ex) {
		dumpError(ex);
	}
}

function loadMap() {
	WebSpec.background = new Image();
	$(WebSpec.background).load(function(){
		
		// Fit to screen * center
		calculateZoomFactor();
		calculateCameraOffsets();
		
		// Get the map config
		$.ajax({
			type: 'GET',
			url: 'maps/'+WebSpec.map+'.txt',
			dataType: 'json',
			success: function(json){
				WebSpec.mapSettings.xoffset = json.xoffset;
				WebSpec.mapSettings.yoffset = json.yoffset;
				WebSpec.mapSettings.flipx = json.flipx;
				WebSpec.mapSettings.flipy = json.flipy;
				WebSpec.mapSettings.scale = json.scale;
				WebSpec.mapSettings.rotation = json.rotation;
				WebSpec.mapSettingsLoaded = true;
			},
			error: function(jqXHR, textStatus) {
				alert("Failed.");
				WebSpec.mapSettingsFailed = true;
			}
		});
	}).error(function(){
		debug("Error loading image");
	}).attr('src', 'maps/'+WebSpec.map+'.jpg');
}

function calculateZoomFactor() {
	//var hRatio = WebSpec.height/WebSpec.background.height;
	//var wRatio = WebSpec.width/WebSpec.background.width;
	WebSpec.cameraZoomFactor = Math.max(WebSpec.height/WebSpec.background.height, WebSpec.width/WebSpec.background.width);
}

function calculateCameraOffsets() {
	WebSpec.cameraXOffset = WebSpec.width/2 - (WebSpec.background.width * WebSpec.cameraZoomFactor /2);
	WebSpec.cameraYOffset = WebSpec.height/2 - (WebSpec.background.height * WebSpec.cameraZoomFactor /2);
}

function mousemove(e)
{
}

function mouseclick(e)
{
}

function debug(msg)
{
	console.log("[debug] "+ msg);
}

function dtor(deg) {
	return deg*(Math.PI/180);
}

function truncateString(ctx, string, width) {
	if (ctx.measureText(string).width <= width)
		return string;
	
	var ellipsisWidth = ctx.measureText("…").width;
	var stringLength = string.length;
	var newStringWidth = 0;
	for (var i = stringLength; i>0; i--) {
		string = string.substring(0, i);
		newStringWidth = ctx.measureText(string).width;
		
		if (newStringWidth <= width-ellipsisWidth)
			return string+"…";
	}
	return "";
}

function dumpError(err) {
  if (typeof err === 'object') {
    if (err.message) {
      console.log('\nMessage: ' + err.message)
    }
    if (err.stack) {
      console.log('\nStacktrace:')
      console.log('====================')
      console.log(err.stack);
    }
  } else {
    console.log('dumpError :: argument is not an object');
  }
}
