library(shiny)
library(shinydashboard)


dashboardPage(
  
  dashboardHeader(title = "Underfloor Heater"),
  
  dashboardSidebar(
    sidebarMenu(
    menuItem("Status", tabName = "status", icon = icon("dashboard")),
    menuItem("Program", tabName = "program", icon = icon("th")),
    menuItem("History", tabName = "history", icon = icon("calendar"))
    ),
    
    textOutput("UserText"),
    textOutput("FlatText"),
    uiOutput("AreaSelector"),
    uiOutput("DeviceSelector")
  ),
  
  dashboardBody(
    tabItems(
      tabItem(tabName = "status",
              h2("Status"),
              p(strong("Current time:"),textOutput("currentTime")),
              uiOutput("SensorSelector"),
              plotOutput("statusPlot")
      ),
      tabItem(tabName = "program",
              h2("Program"),
              fluidRow(
                box(width=6,height=200,
                    uiOutput("UPSelector"),
                    actionButton("UPModify","Modify"),
                    actionButton("UPDelete","Delete")),
                box(width=6,height=200,actionButton("createNewUP","Create new program"))
                
              ),
              fluidRow(
                plotOutput("UPPlot")
              )
      ),
      tabItem(tabName = "history",
              h2("History")
      )
    )
  )
)

