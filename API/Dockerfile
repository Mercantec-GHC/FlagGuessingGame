FROM mcr.microsoft.com/dotnet/sdk:10.0 AS build
WORKDIR /src
COPY FlagGuessingGameAPI.csproj ./
RUN dotnet restore FlagGuessingGameAPI.csproj
COPY . .
RUN dotnet publish FlagGuessingGameAPI.csproj -c Release -o /app/publish --no-restore

FROM mcr.microsoft.com/dotnet/aspnet:10.0 AS runtime
WORKDIR /app
COPY --from=build /app/publish ./
ENV ASPNETCORE_URLS=http://+:5169
EXPOSE 5169
ENTRYPOINT ["dotnet", "FlagGuessingGameAPI.dll"]