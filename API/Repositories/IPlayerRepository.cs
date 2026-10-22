using FlagGuessingGameAPI.Models;
using H2Projekt.Infrastructure.Repositories;

namespace FlagGuessingGameAPI.Repositories
{
    public interface IPlayerRepository : IBaseRepository
    {
        Task<int> AddPlayerAsync(Player player, CancellationToken cancellationToken);
        Task<List<Player>> GetAllPlayersAsync(CancellationToken cancellationToken);
        Task<Player?> GetPlayerAsync(int playerId, CancellationToken cancellationToken);
    }
}