using ArduinoLandeQuizAPI;
using Microsoft.EntityFrameworkCore.Infrastructure;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace ArduinoLandeQuizAPI.Migrations;

[DbContext(typeof(AppDbContext))]
[Migration("20261021120000_AddPlayerName")]
public partial class AddPlayerName : Migration
{
    protected override void Up(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.AddColumn<string>(
            name: "Name",
            table: "Players",
            type: "text",
            nullable: false,
            defaultValue: "");
    }

    protected override void Down(MigrationBuilder migrationBuilder)
    {
        migrationBuilder.DropColumn(name: "Name", table: "Players");
    }
}
