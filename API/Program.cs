using FlagGuessingGameAPI;
using FlagGuessingGameAPI.Repositories;
using FlagGuessingGameAPI.Services;
using Microsoft.EntityFrameworkCore;
using System.Text.Json.Serialization;

var builder = WebApplication.CreateBuilder(args);

// Services
builder.Services.AddProblemDetails();
builder.Services.AddHttpClient();
builder.Services.AddMemoryCache();

// Database
builder.Services.AddDbContext<AppDbContext>(options => options.UseNpgsql(builder.Configuration.GetConnectionString("DefaultConnection")));

// Repositories
builder.Services.AddScoped<IPlayerRepository, PlayerRepository>();

// Country Service
builder.Services.AddSingleton<CountryService>();

// Controllers
builder.Services.AddControllers().AddJsonOptions(options =>
{
    options.JsonSerializerOptions.Converters.Add(new JsonStringEnumConverter());
});

// CORS
builder.Services.AddCors(options =>
{
    options.AddPolicy("AllowAllOrigins", policy =>
        policy.AllowAnyOrigin().AllowAnyMethod().AllowAnyHeader());
});

var app = builder.Build();

app.UseExceptionHandler();
app.UseCors("AllowAllOrigins");

app.MapControllers();

app.Run();