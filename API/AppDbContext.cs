using FlagGuessingGameAPI.Models;
using Microsoft.EntityFrameworkCore;

namespace FlagGuessingGameAPI
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
