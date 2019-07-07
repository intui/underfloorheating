## OpenWeatherMap API

library(httr)
library(lubridate)
library(stringr)
library(plumber)
library(odbc)

configFile <- "config.txt"


# helper function
# easier to put APIKey=... via editor in config.txt
setAPIKey <- function(APIKey) {
  if(file.exists(configFile)) {
    config <- readLines(configFile)
    configNew <- str_replace(config,"APIKey=.*",paste0("APIKey=",APIKey))
    #check if entry exists
    writeLines(configNew, con=configFile)
  } else {
    fileConn <- file(configFile,"w")
    str <- paste0("APIKey=",APIKey)
    writeLines(str, fileConn)
    close(fileConn)
  }
}

# helper function
# easier to put units=... via editor in config.txt
# Fahrenheit (imperial), Celsius (metric) or Kalvin (default)
setUnits <- function(tempUnit) {
  if (tempUnit=="Fahrenheit") units="imperial"
  else if (tempUnit=="Celsius") units="metric"
  else units=""
  
  if(file.exists(configFile)) {
    config <- readLines(configFile)
    configNew <- str_replace(config,"units=.*",paste0("units=",units))
    #check if entry exists
    writeLines(configNew, con=configFile)
  } else {
    fileConn <- file(configFile,"w")
    str <- paste0("units=",units)
    writeLines(str, fileConn)
    close(fileConn)
  }
}

readConfigFile <- function() {
  config <- readLines(configFile)
  temp <- str_extract(config,"APIKey=.*")
  temp <- temp[!is.na(temp)]
  temp <- str_replace(temp,"APIKey=","")
  #assign("APIKey", temp, envir = .GlobalEnv)
  Sys.setenv(APIKEY = temp)
  
  temp <- str_extract(config,"units=.*")
  temp <- temp[!is.na(temp)]
  temp <- str_replace(temp,"units=","")
  Sys.setenv(UNITS = temp)
}

fetchOpenWeatherMapData <- function(zipcode,countrycode="DE",type="now") {
  if (type=="now") urlbase <- "api.openweathermap.org/data/2.5/weather?"
  else if (type=="forecast") urlbase <- "api.openweathermap.org/data/2.5/forecast?"
  query = paste0("zip=",zipcode,",",countrycode)
  #JSON
  apicall <- paste0(urlbase,query,"&APPID=",Sys.getenv("APIKEY"))
  if (Sys.getenv("UNITS")!="") apicall <- paste0(apicall,"&units=",Sys.getenv("UNITS"))
  #XML
  #apicall <- paste0(urlbase,query,"mode=XML&APPID=",APIKey)
  raw.result <- GET(url = apicall)
  if(raw.result$status_code!=200) {
    throw(paste0("Leider nicht erfolgreich, Statuscode: ",raw.result$status_code))
    break
  } 
  if (type=="now") return(content(raw.result))
  return(content(raw.result)$list)
}

getForecastFromOpenWeatherMap <- function(zipcode, countrycode="DE") {
  urlbase <- "api.openweathermap.org/data/2.5/forecast?"
  query = paste0("zip=",zipcode,",",countrycode)
  #JSON
  apicall <- paste0(urlbase,query,"&units=metric&&APPID=",Sys.getenv("APIKEY"))
  #XML
  #apicall <- paste0(urlbase,query,"mode=XML&APPID=",APIKey)
  raw.result <- GET(url = apicall)
  if(raw.result$status_code!=200) {
    throw(paste0("Leider nicht erfolgreich, Statuscode: ",raw.result$status_code))
    break
  } 
  return(content(raw.result)$list)
}

FCToDataframe <- function(FC) {
  TSFunc <- function(x) {
    return(with_tz(as.POSIXct(x$dt,origin=as.Date("1970-01-01"),tz = "UTC"),"CET"))
  }
  return(data.frame(TS = do.call(c,lapply(FC,TSFunc)), 
                    Type = "FC",
                    Temperature = sapply(FC,FUN=function(x) {x$main$temp}), 
                    Humidity = sapply(FC,FUN=function(x) {x$main$humidity}), 
                    Clouds = sapply(FC,FUN=function(x) {x$clouds$all}), 
                    Windspeed = sapply(FC,FUN=function(x) {x$wind$speed}),
                    Rain = sapply(FC,FUN=function(x) {ifelse(is.null(x$rain$`3h`),0,x$rain$`3h`)}),
                    Pressure = sapply(FC,FUN=function(x) {x$main$pressure}),
                    stringsAsFactors = FALSE
        ))
}
   
WeatherNowToDataframe <- function(WeatherNow) {
  return(data.frame(TS = with_tz(as.POSIXct(WeatherNow$dt,origin=as.Date("1970-01-01"),tz = "UTC"),"CET"), 
                    Type = "Real",
                    Temperature = WeatherNow$main$temp, 
                    Humidity = WeatherNow$main$humidity, 
                    Clouds = WeatherNow$clouds$all, 
                    Windspeed = WeatherNow$wind$speed,
                    Rain = ifelse(is.null(WeatherNow$rain$`3h`),0,WeatherNow$rain$`3h`),
                    Pressure = WeatherNow$main$pressure,
                    stringsAsFactors = FALSE
  ))
}





writeDataToDB <- function(Weat,FC) {
  #Dummy-Funktion  
  return()
}



#* @apiTitle updateWeatherForecast
#* gets weather forecast from openweathermap for a given zipcode
#* and writes it to the database (DB not implemented yet)
#* @param zipcode 
#* @param countrycode
#* @post /updateWeatherForecast
updateWeatherForecast <- function(zipcode = 53173, countrycode = "DE") {
  readConfigFile()
  FC <- fetchOpenWeatherMapData(zipcode,countrycode,"forecast")
  FC <- FCToDataframe(FC)
  WeatherNow <- fetchOpenWeatherMapData(zipcode,countrycode,"now")
  WeatherNow <- WeatherNowToDataframe(WeatherNow)
  FC <- rbind(WeatherNow,FC)
  
  writeDataToDatabase(FC)
  return(FC)
}


