using ArduinoLandeQuizAPI.Models;
using Microsoft.EntityFrameworkCore;

namespace ArduinoLandeQuizAPI
{
    public class AppDbContext : DbContext
    {
        public DbSet<Player> Players => Set<Player>();

        public AppDbContext(DbContextOptions<AppDbContext> options) : base(options) { }

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            base.OnModelCreating(modelBuilder);
        }
    }
}
