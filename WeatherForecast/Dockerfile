# Basis-Package
FROM rocker/r-base

# updates
RUN apt-get update -qq && apt-get install -y \
  git-core \
  libssl-dev \
  libcurl4-gnutls-dev

# R-Packages installieren
RUN R -e "install.packages(c('plumber','stringr','httr','lubridate','odbc'))"

# Dateien in Docker-Image kopieren
COPY config.txt /config.txt
COPY WeatherForecastService.R /WeatherForecastService.R

# COPY mysql-connector-odbc-8.0.16-linux-glibc2.12-x86-64bit.tar.gz .

#RUN tar xvzf mysql-connector-odbc-8.0.16-linux-glibc2.12-x86-64bit.tar.gz \
#    && ./mysql-connector-odbc-8.0.16-linux-glibc2.12-x86-64bit/bin/myodbc-installer -a -d -n \
#    "MySQL ODBC 8.0 Driver" -t "Driver=/usr/local/lib/libmyodbc8w.so" 

EXPOSE 8000
#ENTRYPOINT ["R", "-e", "pr <- plumber::plumb(commandArgs()[4]); pr$run(host='0.0.0.0', port=8000)"]
#CMD ["/usr/local/lib/R/site-library/plumber/examples/04-mean-sum/plumber.R"]


ENTRYPOINT ["R", "-e", \
    "pr <- plumber::plumb('WeatherForecastService.R'); pr$run(host='0.0.0.0', port=8000)"]
	
	
# starten:
# docker run -d -p 8000:8000 WeatherForecastService

# Win nutzt nicht localhost, sondern lokale IP. Diese kann über docker-machine ip angezeigt werden

# stoppen:
# ID über docker container ls herausfinden, dann
# docker container stop ID	

# Aufruf z.B.
# curl -X POST "http://192.168.99.100:8000/learn" -H  "accept: application/json"