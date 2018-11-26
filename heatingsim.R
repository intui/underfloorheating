library(lubridate)
library(magrittr)




  
setTemp <- function(Daten,name,degree,startTS=NULL,endTS=NULL) {
  if (!(name %in% colnames(Daten))) {stop(paste0(name," ist kein Spaltenname in Daten"))}
  if(is.null(startTS) & is.null(endTS)) {
    Daten[,name] <- degree
  }
  else if (!is.null(startTS) & !is.null(endTS)) {
    Daten[Daten$TS>=startTS & Daten$TS<=endTS,name] <- degree
  }
  else {
    warning("Bitte geben Sie Start- und Endzeitpunkt an")
  }
  return(Daten)
}

#Daten <- setWaterTempTime(Daten,20,from="00:00",to="06:00")
#name="WaterTemp"
#degree=20
#from="00:00"
#to="06:00"
setTempTime <- function(Daten,name,degree,from="HH:MM",to="HH:MM",weekday=NULL) {
  if (!(name %in% colnames(Daten))) {stop(paste0(name," ist kein Spaltenname in Daten"))}
  if(from!="HH:MM" & to!="HH:MM") {
    #from <- as-numeric(gsub(":",".",from))
    #to <- as-numeric(gsub(":",".",to))
    if(is.null(weekday)) {
      Daten[paste0(sprintf("%02i",hour(Daten$TS)),":",sprintf("%02i",minute(Daten$TS)))>=from & 
              paste0(sprintf("%02i",hour(Daten$TS)),":",sprintf("%02i",minute(Daten$TS)))<=to,name] <- degree
    }
    else {
      Daten[as.POSIXlt(Daten$TS)$wday==weekday & paste0(sprintf("%02i",hour(Daten$TS)),":",sprintf("%02i",minute(Daten$TS)))>=from & 
              paste0(sprintf("%02i",hour(Daten$TS)),":",sprintf("%02i",minute(Daten$TS)))<=to,name] <- degree
    }
  }
  else {
    warning("Bitte geben Sie Start- und Endzeit an")
  }
  return(Daten)
}


setWaterTemp <- function(Daten,degree,startTS=NULL,endTS=NULL) {
  return(setTemp(Daten,"WaterTemp",degree,startTS,endTS))
}

setWaterTempTime <- function(Daten,degree,from="HH:MM",to="HH:MM",weekday=NULL) {
  return(setTempTime(Daten,"WaterTemp",degree,from,to,weekday))
}

setTargetTemp <- function(Daten,degree,startTS=NULL,endTS=NULL) {
  return(setTemp(Daten,"TargetTemp",degree,startTS,endTS))
}

setTargetTempTime <- function(Daten,degree,from="HH:MM",to="HH:MM",weekday=NULL) {
  return(setTempTime(Daten,"TargetTemp",degree,from,to,weekday))
}


# Sensor auslesen 
readRoomTemp <- function(Daten,TS=NULL) {
  if(is.null(TS)) {
    #finde frühesten Zeitpunkt mit NA
    #besser wäre allerdings frühester Zeitpunkt, ab dem nur noch NAs kommen
    TS <- Daten$TS[min(which(is.na(Daten$RoomTemp)))]
  }
  Daten$RoomTemp[Daten$TS==TS] <- 16
  return(Daten)
}

lambda <- function(degree,WaterTemp,delta=10) {
  return(ifelse(degree<(WaterTemp-delta),1,
         ifelse(degree>=WaterTemp,0,-1/delta*degree + WaterTemp/delta
         )))
}


#heaterParams <- heaterLivingroom
#nowTS <- Daten$TS[1]
heat <- function(Daten,nowTS,heaterParams) {
  #update Daten, 
  rowNr <- which(Daten$TS==nowTS)
  Daten$Heating[rowNr] <- 1
  
  steps <- (-2*heaterParams$standdev):(2*heaterParams$standdev)
  #timeWindow <- Daten$TS[rowNr+heaterParams$delay+steps]
  timeWindow <- rowNr+heaterParams$delay+steps
  values <- lambda(Daten$forecastRoomTemp[timeWindow],Daten$WaterTemp[timeWindow],heaterParams$tempDelta)*
    heaterParams$maxFactor*dnorm(steps,sd=heaterParams$standdev)

  #Wichtig: Darf nicht mehr NA sein, da additiv
  Daten$forecastRoomTemp[timeWindow] <- Daten$forecastRoomTemp[timeWindow]+values
  
  return(Daten)
}




updateForecastRoomTemp <- function(Daten,nowTS) {
  #hier passiert der 1. interessante Teil
  #Wir wollen die Raumtemperatur prognostizieren
  #1. äußere Einflüsse
  
  #(2. Heizen) passiert beim Heizen
  
  
  #dummy: einfach aktuelle Temperatur fortschreiben, sofern noch nicht gesetzt
  Daten$forecastRoomTemp[Daten$TS>nowTS & is.na(Daten$forecastRoomTemp)] <- Daten$RoomTemp[Daten$TS==nowTS]
  return(Daten)  
}


evalGoodness <- function(Daten,fromTS,toTS,forecast=FALSE) {
  if(forecast) {
    return(abs(sum(Daten$forecastRoomTemp[Daten$TS>=fromTS & Daten$TS<=toTS]-Daten$TargetTemp[Daten$TS>=fromTS & Daten$TS<=toTS],na.rm=TRUE)))
  } else {
    return(abs(sum(Daten$RoomTemp[Daten$TS>=fromTS & Daten$TS<=toTS]-Daten$TargetTemp[Daten$TS>=fromTS & Daten$TS<=toTS],na.rm=TRUE)))
  }
}


#nowTS <- Daten$TS[241]

updateHeating <- function(Daten,nowTS,heaterParams) {
  #hier passiert der 2. interessante Teil
  #Passe das Heizverhalten an, damit sich Prognose der TargetTemp annähert
  temp <- heat(Daten,nowTS,heaterParams)
  off <- evalGoodness(Daten,fromTS=nowTS,toTS=nowTS+7200,forecast=TRUE)
  on <- evalGoodness(temp,fromTS=nowTS,toTS=nowTS+7200,forecast=TRUE)
  if(on<off) {
    Daten <- temp
  }
  
  
  nowNr <- which(Daten$TS==nowTS)
  if(Daten$Heating[nowNr]==1 & Daten$Heating[max(1,nowNr-1)]==0) {
    print(paste0(nowTS,": Turn heater on"))
  } else if (Daten$Heating[nowNr]==0 & Daten$Heating[max(1,nowNr-1)]==1) {
    print(paste0(nowTS,": Turn heater off"))
  }
  
  return(Daten)  
}


# delay = delay till heating kicks in (in frequency)
# tempDelta = parameter for lambda-function. How many degrees unter watertemp the slowing kicks in
# maxDelta = maximal degree delta for one step heating
# standdev = how slow is one step heating (width of normal distribution)
setupHeater <- function(roomName,delay=10,tempDelta=10,maxDelta=1,standdev=2) {
  l <- list(
    "roomName"=roomName,
    "delay"=delay,
    "tempDelta"=tempDelta,
    "maxDelta"=maxDelta,
    "maxFactor"=maxDelta/dnorm(0,sd=standdev),
    "standdev"=standdev  
  )
  return(l)
}


## Example of a simulation ###################################################################

#Setup

# Start and Enddate of simulation
startDate <- as.Date("2018-11-22")
endDate <- as.Date("2018-12-21")

# Update-frequency in seconds
Frequenz <- 60

# create empty dataframe
n <- ((as.numeric(endDate - startDate)+1)*24*60*60 / Frequenz)-1
startTS <- ymd_hms(paste0(startDate," 00:00:00"))
Daten <- data.frame(ID=0:n,TS=startTS+Frequenz*0:n,
                    WaterTemp=NA,RoomTemp=NA,TargetTemp=NA,forecastRoomTemp=NA,Heating=0,stringsAsFactors=FALSE)
rm(startTS)

# parameters of heating in this room
heaterLivingroom <- setupHeater("Living Room",delay=10,tempDelta=10,maxDelta=1,standdev=2)


# "program" watertemperatur
Daten <- Daten %>% setWaterTempTime(20,from="00:00",to="06:00") %>%
                   setWaterTempTime(33,from="06:00",to="22:00") %>% 
                   setWaterTempTime(20,from="22:00",to="00:00")

# "program" target temperature
Daten <- Daten %>% setTargetTempTime(16,from="00:00",to="06:00") %>%
                   setTargetTempTime(21,from="06:00",to="22:00") %>% 
                   setTargetTempTime(16,from="22:00",to="00:00")








# Loop
for (i in 1:n) {
  if(minute(Daten$TS[i])==0) print(Daten$TS[i])
  
  # 1. Sensor auslesen
  Daten <- readRoomTemp(Daten)
  
  # 2. Prognose updaten
  Daten <- updateForecastRoomTemp(Daten,Daten$TS[i])
  
  # 3. Heizverhalten anpassen
  Daten <- updateHeating(Daten,Daten$TS[i],heaterLivingroom)
}







showChart <- function(Daten,fromTS=NULL,toTS=NULL,KPI="RoomTemp") {
  if(is.null(fromTS)) fromTS <- Daten$TS[0]
  if(is.null(toTS)) toTS <- Daten$TS[nrow(Daten$TS)]
  plot(Daten$TS[Daten$TS>=fromTS & Daten$TS<=toTS],Daten[Daten$TS>=fromTS & Daten$TS<=toTS,KPI],
       xlab="Zeit",ylab="Temperatur",main=KPI)
}

showChart(Daten,as.Date("2018-11-22"),as.Date("2018-11-23"))
showChart(Daten,as.Date("2018-11-22"),as.Date("2018-11-24"),"TargetTemp")
